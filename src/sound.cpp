#include "sound.h"
#include <Arduino.h>>

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
        return;
    }

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
    // todo
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
