#include "analogs.h"
#include "defs.h"
#include <LiPoBattery.h>
#include <stdint.h>

uint16_t _battmv;
uint16_t _battPercent;
uint16_t _chargemA;
uint32_t _battMvAccum;
uint8_t _accumSamples;
uint32_t _chmaAccum;

static const uint32_t refmv    = 2500;
static const uint16_t adcScale = 1023;
static const uint16_t Rch      = 3300;

static const uint8_t ACCUMBITS = 4;
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

    if(first)
    {
        _battMvAccum =0; 
        _chmaAccum =0;
    }

    //
    // Battery volts
    analogReference(INTERNAL2V5);
    uint16_t raw = analogRead(Pin::VBATT);
    auto battmv = 2*raw*refmv/adcScale;   // we have a 0.5 divider
    _battMvAccum += battmv;

    raw = analogRead1(Pin::ISENSE);
    uint32_t chargemA = (raw*refmv);
    chargemA /= adcScale;
    chargemA *= 1000;
    chargemA /= Rch;
    _chmaAccum += chargemA;

    ++_accumSamples;
    if(first)
    {
        // just one sample
        _battPercent =  LiPoMvToPercent(battmv);
        _battmv = battmv;
        _chargemA = chargemA;
        ret = true;
        //Serial.printf("Volts raw=%u mV=%u percent=%u\r\n", raw, _battmv, _battPercent);
    }
    else if(_accumSamples == (1<<ACCUMBITS))
    {
        _battMvAccum += (1<<(ACCUMBITS-1));
        _chmaAccum += (1<<(ACCUMBITS-1));
        _battMvAccum >>= ACCUMBITS;
        _chmaAccum >>= ACCUMBITS;

        _battPercent =  LiPoMvToPercent(_battMvAccum );
        _battmv = _battMvAccum;
        //Serial.printf("Volts accum mV=%lu percent=%u\r\n", _battMvAccum, _battPercent);
        _accumSamples = 0;
        _battMvAccum = 0;


        ret = true;
    } 
    else
    {
        //Serial.printf("Volts accum(%u)  mV=%lu \r\n", _accumSamples, _battMvAccum);
    }



    //auto chsen = analogRead(Pin::CHG_SENSE);

    //Serial.printf("Ichg raw=%u mA=%u\r\n", raw, _chargemA);

    return ret;
}
//-------------------------------------------------------


