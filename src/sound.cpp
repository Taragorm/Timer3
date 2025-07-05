#include "sound.h"
#include <Arduino.h>>
#include "waves.h"
#include "defs.h"

#define TMR TCB0
#define TMRVECT TCB0_INT_vect
#define DAC DAC0

static const uint8_t SAMPLES = 16;
static uint8_t _soundBuffer[SAMPLES] =
{
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
};
static uint8_t* _workPtr;
static const uint8_t* _endPtr = _soundBuffer+SAMPLES;

static ISound* _currentSound;

static ISound* _sounds[MAXSOUND];


//------------------------------------------
bool isPlayingSound()
{
    return _currentSound != 0;
}
//------------------------------------------
void playSound(int index)
{

    if(index<0 || index>=MAXSOUND)
    {
        //Serial.printf("Sound ix=%d - off\r\n", index);
        _currentSound = nullptr;
    }
    else
    {
        _currentSound = _sounds[index];
        //Serial.printf("Sound ix=%d = %x\r\n", index, _currentSound);
    }

    if(_currentSound)
    {
        //Serial.printf("Sound on\r\n");
        digitalWriteFast(Pin::SOUND_EN, 1);
        _currentSound->init(true);
    }
    else
    {
        //Serial.printf("Sound off\r\n");
        digitalWriteFast(Pin::SOUND_EN, 0);
        setFreq(0);
    }
}
//------------------------------------------
constexpr uint16_t computeCounts(uint16_t freq)
{
    // assumes CLK/2 drive
    return F_CPU/(2L*SAMPLES*freq);
}
//------------------------------------------
void setFreq(uint16_t f)
{
    if(f==0)
    {
        // just turn it off
        TMR.CTRLA &= ~ TCB_ENABLE_bm;
        digitalWriteFast(Pin::SOUND_EN, 0);
        return;
    } if(f>2000)
        f = 2000;

    TMR.CCMP = computeCounts(f);
    //Serial.printf("counts=%u\r\n", TMR.CCMP);
    TMR.CTRLA |=  TCB_ENABLE_bm;
}
//------------------------------------------
/**
 * Init the timer system
 */
void soundInit()
{
    _workPtr = _soundBuffer;

    DAC.CTRLA = DAC_ENABLE_bm|DAC_OUTEN_bm;

    TMR.CTRLA = TCB_CLKSEL_CLKDIV2_gc;
    TMR.CTRLB = 0; // periodic int mode
    TMR.INTCTRL = TCB_CAPT_bm;
}
//------------------------------------------
void soundSleep()
{
    TMR.CTRLA   &= ~TCB_ENABLE_bm;
    TMR.INTCTRL &= ~TCB_CAPT_bm;
    DAC.CTRLA   &= ~DAC_ENABLE_bm;
}
//------------------------------------------
/**
 * Kick the timer for time-related functions
 */
void soundLoop()
{
    if(_currentSound)
    {
        auto was = _currentSound;
        _currentSound = _currentSound->execute();
        if(was != _currentSound)
        {
            if(_currentSound)
                _currentSound->init(false);
            else
                setFreq(0);
            //Serial.printf("sound gen change %x ->  %x\r\n", was, _currentSound);
        }
    }
}
//------------------------------------------
ISR(TMRVECT) 
{
    DAC.DATA     = *_workPtr++;
    if(_workPtr==_endPtr)
    {
        _workPtr = _soundBuffer;
    }

    TMR.INTFLAGS = TCB_CAPT_bm;
}
//------------------------------------------
ISound* ISound::makeCircular()
{
    ISound* last = this;
    while(last->_next != nullptr)
    {
        //Serial.printf("    %x->next = %x\r\n", last, last->_next);
        last = last->_next;
    }

    last->_next = this;    
    //Serial.printf("end %x->next = %x\r\n", last, this);

    return this;
}
//==========================================
class Bleep : public ISound
{
    const uint16_t _freq;
    const uint16_t _time;
    uint16_t _count;

public:

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Bleep(uint16_t f, uint16_t t, ISound* next = nullptr)
    : ISound(next), _freq(f), _time(t)
    {}
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    virtual ISound* execute()
    {
        if(--_count==0)
        {
            return _next;
        }
        return this;
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    virtual void init(bool first)
    {
        _count = _time;
        setFreq(_freq);
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
};
//==========================================
class Slide : public ISound
{
    const int16_t _freq;
    const int16_t _time;
    const int16_t _shift;

    int16_t _count;
    int16_t _workFreq;

public:

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Slide(
            int16_t f,
            int16_t f2,
            int16_t t, 
            ISound* next = nullptr
            )
    :   ISound(next), 
        _freq(f), 
        _time(t), 
        _shift((f2-f)/t)
    {}
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    virtual ISound* execute()
    {
        if(--_count==0)
        {
            return _next;
        }
        else
        {
            _workFreq += _shift;
            //Serial.printf("shift %d\r\n", _workFreq);
            setFreq(_workFreq);
            return this;
        }
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    virtual void init(bool first)
    {
        _count = _time;
        _workFreq = _freq;
        //Serial.printf("init %d::%d\r\n", _workFreq, _shift);
        setFreq(_freq);
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
};

//==========================================
static ISound* makeCircular(ISound * sp)
{
    sp->makeCircular();
    return sp;
}
//------------------------------------------
/**
 * Initialise the sound structures
 */
void soundStaticInit()
{
    memcpy(_soundBuffer, triangle, sizeof _soundBuffer );
    _sounds[0] = new Bleep(800, 10);

    _sounds[1] = makeCircular(  
                    new Bleep(800, 20,
                        new Bleep(400, 20)
                    )
                );

    _sounds[2] = makeCircular(  
                    new Bleep(800, 20,
                        new Bleep(400, 20,
                            new Bleep(200, 20)
                        )
                    )
                );
    
    _sounds[3] = makeCircular(  
                    new Bleep(200, 20,
                        new Bleep(400, 20,
                            new Bleep(800, 20)
                        )
                    )
                );

    _sounds[4] = makeCircular(  
                    new Slide(200, 800,  20)
                );

    _sounds[5] = makeCircular(  
                    new Slide(800, 200,  20)
                );

    _sounds[6] = makeCircular(  
                    new Slide(200, 800,  20,
                        new Slide(800, 200,  20)
                    )
                );
                
}
//------------------------------------------
