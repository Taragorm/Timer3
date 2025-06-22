//
// Includes - if we include *everything* here, we don't need deep search enabled.
//
#include <Arduino.h>
#include "defs.h"
#include "SPI.h"
#include <MilliTimer.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>
//#include <psiiot.h>


//Arduino_ESP32SPI* bus = new Arduino_ESP32SPI(Pin::LcdA0, Pin::LcdCsN, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);
//Arduino_DataBus *bus = new Arduino_ESP32SPI( /*dc*/ Pin::LcdA0, /*cs*/ Pin::LcdCsN /*spi_num:VSPI*/ );


//   Adafruit_SH1106G(uint16_t w, uint16_t h, SPIClass *spi, int16_t dc_pin,
//                    int16_t rst_pin, int16_t cs_pin,
//                    uint32_t bitrate = 8000000UL);


 Adafruit_SH110X* gfx = new Adafruit_SH1106G(
    //bus, 
    //Pin::None, 2 /* rotation */,
    128 /* width */, 160 /* height */,
    &SPI,
    Pin::LcdA0, Pin::None, Pin::LcdCsN
    );

DisplayTask* displayTask_ = new DisplayTask(gfx);


MilliTimer pollTimer_(LOCAL_INTERVAL_MS, /*cyclic=*/ false);

MilliTimer publishTimer_(PUBLISH_INTERVAL_MS, /*cyclic=*/ true);


//-----------------------------------------------------------------
void setup()
{
    //pinMode(Pin::LEDTx, OUTPUT);
    //digitalWrite(Pin::LEDTx, 1);
    //pinMode(Pin::LEDRx, OUTPUT);
    //digitalWrite(Pin::LEDRx, 1);


    Serial.begin(115200);
    delay(4000);
    Serial.println("Timer3");
    delay(100);

    Serial.println("GFX begin");
    Serial.println("GFX fill");
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE,BLACK);
    gfx->println(NODENAME);



    Serial.println("Init Done");


}

//-----------------------------------------------------------------
void loop()
{
}
//-----------------------------------------------------------------

//-----------------------------------------------------------------
