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
            400000UL       // bitrate
            );

 
RtcMilliTimer timer_(20, /*cyclic=*/ true);
RtcMilliTimer slowTimer_(1000, /*cyclic=*/ true);

static void keyActions(uint16_t k);
static void power(bool state);
static void displayInit();
static void sleep();
static void mainActions();

bool _powered;


//-----------------------------------------------------------------
void setup()
{
    pinMode(Pin::SOUND_EN, OUTPUT);
    digitalWriteFast(Pin::SOUND_EN, 1);

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
    
    analogInit();

    displayInit();

    soundInit();
    setFreq(0);

    Serial.println("Init Done");

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
}
//-----------------------------------------------------------------
void loop()
{

    if(timer_.isExpired())
    {
        _keypad.scan();
        // //_keypad.dumpKeyMap();


        for(;;)
        {
            auto res = _keypad.keys();
            if(res== Kbd4017Rpt::NOKEY)
                return;            
            Serial.printf("%04x\r\n", res);
            keyActions(res);
        }
    }

    if(slowTimer_.isExpired())
    {
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
            if(state==0x51)
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
            // bleep
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
    }
    else
    {
        digitalWriteFast(Pin::SOUND_EN, false);
        setFreq(0);

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
    _gfx.printf("Batt %u%%\n", _battPercent );

    auto tp = _timers;
    for(uint8_t i=0; i<TIMERCOUNT; ++i, ++tp)
    {
        tp->tick();
        
        _gfx.printf("T%d:%u*%3u", 
                    i+1, 
                    tp->timer.times,
                    tp->timer.mins
                    );

        _gfx.print("\n");                     
    }

    _gfx.display();
}
//-----------------------------------------------------------------
