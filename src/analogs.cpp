#include "analogs.h"
#include "defs.h"
#include <LiPoBattery.h>
#include <stdint.h>

uint16_t _battmv;
uint16_t _battPercent;
uint16_t _chargemA;
uint32_t _battMvAccum;
uint8_t _accumSamples;

static const uint32_t refmv    = 2500;
static const uint16_t adcScale = 1023;
static const uint16_t Rch      = 3300;

//-------------------------------------------------------
void analogInit()
{
    analogReference(INTERNAL2V5);
    init_ADC1();
    analogReference1(INTERNAL2V5);
}
//-------------------------------------------------------
bool readAnalogs(bool first)
{
    bool ret = false;

    //
    // Battery volts
    analogReference(INTERNAL2V5);
    uint16_t raw = analogRead(Pin::VBATT);
    _battmv = 2*raw*refmv/adcScale;   // we have a 0.5 divider

    _battMvAccum += _battmv;
    ++_accumSamples;
    if(first)
    {
        _battPercent =  LiPoMvToPercent(_battmv);
        ret = true;
        Serial.printf("Volts raw=%u mV=%u percent=%u\r\n", raw, _battmv, _battPercent);
    }
    else if(_accumSamples==128)
    {
        _battMvAccum >>= 7;

        _battPercent =  LiPoMvToPercent(_battMvAccum );
        Serial.printf("Volts accum mV=%lu percent=%u\r\n", _battMvAccum, _battPercent);
        _accumSamples = 0;
        _battMvAccum = 0;
        ret = true;
    } 
    else
    {
        //Serial.printf("Volts accum(%u)  mV=%lu \r\n", _accumSamples, _battMvAccum);
    }


    return ret;

    // raw = analogRead1(Pin::ISENSE);
    // _chargemA = (1000000*raw*refmv)/(Rch*adcScale);
    // Serial.printf("Ichg raw=%u mA=%u\r\n", raw, _chargemA);
}
//-------------------------------------------------------


