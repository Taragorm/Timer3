#include "fsm.h"
#include "defs.h"
#include "timers.h"

static char base(char ch);
static char getTmr(char ch, char min);
static char getEditTmr(char ch);
static char getNumber(char ch, uint8_t digits);
static char getMins(char ch);
static char getTimes(char ch);
static char beginNum(const char* prompt);
static char timerFunc(char ch, void (Timer::*fn)());

char (*fsm)(char ch) = base;
char (*fsmNext)(char ch);

static uint8_t timerNo;

const char lastChanChar = '0' + TIMERCOUNT;
const uint8_t promptRow = 7*8;
static char numbuff[8];
static uint16_t mins;
static char* endPtr;

#define CALL_MEMBER_FN(object, ptrToMember)  ((object).*(ptrToMember))

static const char* promptFmt;

//---------------------------------------------------------
void resetFsm()
{
    fsm = base;
}
//---------------------------------------------------------
void clearPrompt()
{
    _gfx.fillRect(0, promptRow, 128, 8, BLACK);
    _gfx.display();
}
//---------------------------------------------------------
static char base(char ch)
{
    _gfx.setCursor(0,promptRow);

    switch(ch)
    {
        case '<':
            _gfx.printf("Reset 1..%c 0=All", lastChanChar);
            fsm=[](char ch) -> char { return timerFunc(ch, &Timer::rewind); };
            break;

        case '|':
            _gfx.printf("Pause 1..%c 0=All", lastChanChar);
            fsm=[](char ch) -> char { return timerFunc(ch, &Timer::stop); };
            break;

        case '>':
            _gfx.printf("Start 1..%c 0=All", lastChanChar);
            fsm=[](char ch) -> char { return timerFunc(ch, &Timer::start); };
            break;

        // case 'P':
        //     break;

        case 'X':
            _gfx.printf("Clear 1..%c 0=All", lastChanChar);
            fsm=[](char ch) -> char { return timerFunc(ch, &Timer::clear); };
             break;

        case 'C': // COG = edit
            _gfx.printf("Edit 1..%c", lastChanChar);
            fsm = getEditTmr;
            break;
 
        default:
            return 1;
    }

    _gfx.display();
    return 0;
}
//---------------------------------------------------------
static char getTmr(char ch, char min)
{
    clearPrompt();
    if(ch>='0'+min && ch <= lastChanChar)
    {
        timerNo = ch-'0';
        return timerNo;
    }
    else
    {
        return -1;
    }
}
//---------------------------------------------------------
static char getEditTmr(char ch)
{
    auto res = getTmr(ch,1);
    if(res>0)
    {
        // got a valid timer in \a channel
        beginNum("Mins? %s");
        fsm = getMins;    
        return 0;    
    }

    // not ok
    return 1;
}
//---------------------------------------------------------
static char getNumber(char ch, uint8_t digits)
{
    bool done = false;

    if(ch>='0'&&ch<='9')
    {
        *endPtr++ = ch;
    }
    else
    {
        switch(ch)
        {
            case '<': // delete
                if(endPtr==numbuff)
                {
                    fsm = base;
                    clearPrompt();
                    return 0;
                }
                else
                {
                    --*endPtr = 0;
                }

            case '>': // done
                if(endPtr==numbuff)
                {
                    // nothing entered
                    fsm = base;
                    clearPrompt();
                    return 1;
                }
                done = true;
                break;

            case 'X':
                fsm = base;
                clearPrompt();
                return 0;

        }
    }

    if( done || endPtr >= numbuff+digits)
    {
        // we done
        fsm = fsmNext;
        clearPrompt();
        return -1;
    }

    _gfx.setCursor(0,promptRow);
    _gfx.printf(promptFmt, numbuff);
    _gfx.display();
    return 0;
}
//---------------------------------------------------------
static char getMins(char ch)
{
    char res = getNumber(ch,3);
    if(res==-1)
    {
        mins = (unsigned)atoi(numbuff);
        if(mins==0)
        {
            return 1;
        }

        //
        // otherwise, get repeats
        fsm = getTimes;
        beginNum("Times? %s");
    }
    return 0;
}
//---------------------------------------------------------
static char getTimes(char ch)
{
    char res = getNumber(ch,1);
    if(res==-1)
    {
        auto times = (unsigned)atoi(numbuff);
        if(mins==0)
        {
            fsm = base;
            return 1;
        }

        //
        // done
        auto tp = _timers+timerNo-1;

        //
        // what to do if running?
        tp->timer.mins = mins;
        tp->timer.times = times;

        clearPrompt();

        fsm = base;
    }
    return 0;
}
//---------------------------------------------------------
static char edit(char ch)
{
}
//---------------------------------------------------------
static char beginNum(const char* prompt)
{
    memset(numbuff, 0, sizeof numbuff);
    endPtr = numbuff;
    promptFmt = prompt;
    _gfx.setCursor(0,promptRow);
    _gfx.printf(prompt,numbuff);
}
//---------------------------------------------------------
static char timerFunc(char ch, TimerMemFn fn)
{
    auto t = getTmr(ch,0);
    if(t==-1) return 1;
    if(t==0)
    {
        for(t=0; t<TIMERCOUNT; ++t)
            CALL_MEMBER_FN(_timers[t], fn)();
    }
    else
        CALL_MEMBER_FN(_timers[t-1], fn)();
    
    fsm=base;
    return 0;
}
//---------------------------------------------------------
