//
// Includes - if we include *everything* here, we don't need deep search enabled.
//
#include <Arduino.h>
#include <avr/sleep.h>
#include "defs.h"
#include "SPI.h"
#include <MilliTimer.h>
#include <Wire.h>
#include <SPI.h>
#include <Kbd4017Rpt.h>
#include <Adafruit_SSD1306.h>
#include "sound.h"
#include "analogs.h"
#include "timers.h"
#include "fsm.h"

const FastPin _kbClk(PORTB, 1);
const FastPin _kbRst(PORTB, 4);

const FastPin _rows[2] =
{
    FastPin(PORTC, 2),
    FastPin(PORTC, 3)
};

/** Map key codes to readable chars */
const char _keys[] = "<|>P123X456C7890";

Kbd4017Rpt _keypad(
            _kbClk,
            _kbRst,
            _rows, 2,
            8, // columns
            _keys
            );


Adafruit_SSD1306 _gfx(
            128,            // xw
            64,             // yh 
            &SPI,
            Pin::DISPL_DC,
            Pin::NONE,      // rst
            Pin::DISPL_CS, 
            8000000UL       // bitrate
            );

 
RtcMilliTimer timer_(20, /*cyclic=*/ true);
RtcMilliTimer slowTimer_(1000, /*cyclic=*/ true);

SharedTickTimer _shutdownTimer(5*60, false);
SharedTickTimer _silenceTimer(60, false);

static void keyActions(uint16_t k);
static void power(bool state);
static void displayInit();
static void sleep();
static void mainActions();

bool _powered;

//-----------------------------------------------------------------
static void testBattLow()
{
    readAnalogs(true);
    if(_battPercent<30)
    {
        _gfx.printf("LOW BATTERY %u%%", _battPercent);
        _gfx.display();

        setFreq(440);
        delay(500);
        setFreq(0);
    }
}
//-----------------------------------------------------------------
void setup()
{

    pinMode(Pin::SOUND_EN, OUTPUT);
    //digitalWriteFast(Pin::SOUND_EN, 1);

    pinMode(Pin::SOUND, OUTPUT);
    //digitalWrite( Pin::SOUND, true );

    pinMode(Pin::POWER_SW, OUTPUT);
    delay(250);
    digitalWriteFast(Pin::POWER_SW, 1);
    _powered = true;
    delay(250);

    Serial.begin(115200);
    delay(250);
    Serial.println("Timer3");
    delay(100);
    soundStaticInit();
    
    analogInit();

    //_gfx.setFont((const GFXfont*) font);
    displayInit();

    soundInit();
    setFreq(0);

    testBattLow();
    //Serial.println("Init Done");


}
//-----------------------------------------------------------------
static void displayInit()
{
    Serial.println("GFX begin");
    if(!_gfx.begin(SSD1306_SWITCHCAPVCC)) 
    {
      Serial.println(F("SH1106 allocation failed"));
    }
    //delay(250);
    _gfx.clearDisplay();
    _gfx.setCursor(0,0);
    _gfx.setTextColor(1,0);
    _gfx.dim(true);
}
//-----------------------------------------------------------------
void loop()
{

    if(timer_.isExpired())
    {
        soundLoop();

        _keypad.scan();

        for(;;)
        {
            auto res = _keypad.keys();
            if(res== Kbd4017Rpt::NOKEY)
                return;            

            _shutdownTimer.reset();
            keyActions(res);
        }
    }

    if(slowTimer_.isExpired())
    {
        SharedTickTimer::tick();
        mainActions();
    }

}
//-----------------------------------------------------------------
static void keyActions(uint16_t kc)
{
    char key = kc & Kbd4017Rpt::KEYMASK;
    char state = (kc & Kbd4017Rpt::RPTMASK) >> 8;
    switch(key)
    {
        case 'P':
            if(state==0)
                playSound(-1);
            else if(state==0x51)
                sleep();
            break;
    }

    if(state==0)
    {
        auto err = fsm(key);
        if(err)
        {
            resetFsm();
            clearPrompt();
            playSound(0);
        }
        else
        {
            _gfx.display();
        }        
    }
}
//-----------------------------------------------------------------
static void sleep()
{
    Serial.print("Begin sleep\r\n");
    delay(100);
    power(false);

    // set up keypad so we're scanning the power button
    _keypad.setColumn(1);

    //
    // wait for key to be released for a bit
    RtcMilliTimer tmr(100, false);
    for(;;)
    {
        if( digitalReadFast(Pin::KBD1) )
        {
            tmr.reset();
        } else if(tmr.isExpired())
            break;
    }

    // enable interrupt so we can wake
    PORTC.INTFLAGS = (1<<3);
    PORTC.PIN3CTRL = PORT_ISC_BOTHEDGES_gc;  // PORT_ISC_RISING_gc does not work for wake

    Serial.print("Sleeping\r\n");
    delay(100);

    // sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();

    Serial.print("Wake\r\n");

    //
    // once here, we're awake again
    PORTC.PIN3CTRL = PORT_ISC_INTDISABLE_gc;
    
    //PORTC.INT
    power(true);
    delay(100);

    testBattLow();

}
//-----------------------------------------------------------------
static void power(bool state)
{
    Serial.printf("Power %d\r\n",state);
    auto tri = state ? OUTPUT : INPUT_PULLUP;
    pinMode(Pin::SCK, tri);
    digitalWriteFast(Pin::POWER_SW, state);                
    _powered = state;
    if(state)
    {        
        displayInit();        
        init_ADC0();
        init_ADC1();        
        _shutdownTimer.reset();
        _silenceTimer.reset();
    }
    else
    {
        digitalWriteFast(Pin::SOUND_EN, false);
        playSound(-1);

        // disable the ADCs
        ADC0.CTRLA = 0;
        ADC1.CTRLA = 0;
    }
}
//-----------------------------------------------------------------
ISR(PORTC_PORT_vect)
{
    // we just wake the CPU
    PORTC.INTFLAGS = (1<<3);
    Serial.print("#");
}
//-----------------------------------------------------------------
static void mainActions()
{
    readAnalogs(false);
    _gfx.fillRect(0,0,128,7*8, BLACK); // exclude the prompt line

    _gfx.setCursor(0,0);
    _gfx.printf("Batt %u%%", _battPercent );
    if(_chargemA>4)
        _gfx.printf(" %umV %umA", _battmv, _chargemA );
    _gfx.println();

    bool running = false;
    auto tp = _timers;
    for(uint8_t i=0; i<TIMERCOUNT; ++i, ++tp)
    {
        if(!tp->isConfigured())
            continue;

        if(tp->running)
        {
            //Serial.printf("T%d running\r\n", i);
            running = true;
        }

        if(tp->tick())
        {
            playSound(i+1);
        }
        
        _gfx.printf("T%d %u*%03u %", 
                    i+1, 
                    tp->timer.times,
                    tp->timer.mins                    
                    );

        _gfx.write(tp->stateChar());

        if(tp->isConfigured())
        {
            _gfx.printf(" %d*%03um%02d", 
                    tp->times,
                    tp->mins,
                    tp->secs
                    );
        }

        _gfx.print("\n");                      
    }

    _gfx.display();

    //
    // shutup after a minute
    if(isPlayingSound())
    { 
        if(_silenceTimer.isExpired())
            playSound(-1);
    }
    else
        _silenceTimer.reset();

    //
    // shutdown if nothing is happening
    //Serial.printf("sdt=%d  tb=%d\r\n", _shutdownTimer.intervalExpired(), _shutdownTimer.ms());

    if(running)
        _shutdownTimer.reset();
    else
    {
        if(_shutdownTimer.isExpired())
        {
            sleep();
        }
    }
}
//-----------------------------------------------------------------
