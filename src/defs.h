/*
 * io.h
 *
 * Created: 13/03/2021 13:36:52
 *  Author: Simon
 */


#ifndef IO_H_
#define IO_H_

#include <Arduino.h>
#include <stdint.h>
#include <Adafruit_SSD1306.h>


// ATtiny1616 / ARDUINO
//                          _____
//                  VDD   1|*    |20  GND
// (nSS)  (AIN4) PA4  0~  2|     |19  16~ PA3 (AIN3)(SCK)(EXTCLK)
//        (AIN5) PA5  1~  3|     |18  15  PA2 (AIN2)(MISO)
// (DAC)  (AIN6) PA6  2   4|     |17  14  PA1 (AIN1)(MOSI)
//        (AIN7) PA7  3   5|     |16  17  PA0 (AIN0/nRESET/UPDI)
//        (AIN8) PB5  4   6|     |15  13  PC3
//        (AIN9) PB4  5   7|     |14  12  PC2
// (RXD) (TOSC1) PB3  6   8|     |13  11~ PC1 (PWM only on 1-series)
// (TXD) (TOSC2) PB2  7~  9|     |12  10~ PC0 (PWM only on 1-series)
// (SDA) (AIN10) PB1  8~ 10|_____|11   9~ PB0 (AIN11)(SCL)
//
//

/*  0/1-series. ADC1, TCB1 and AC1, AC2 present only on 16/32k 1-series. TCD0, DAC0, alternate SPI/I2C pins and AC0 inputs past 0 only available on 1-series.

  PIN#   DESC         Pin Name  Other/Sp  ADC0      ADC1      PTC       AC0       AC1       AC2       DAC0      USART0    SPI0      TWI0      TCA(PWM)  TCBn      TCD0      CCL
  0                   PA4                 AIN4      AIN0      X0/Y0                                              XDIR      SS                  WO4                WOA        LUT0-OUT
  1                   PA5       VREFA     AIN5      AIN1      X1/Y1     OUT       AINN0                                                        WO5       TCB0 WO  WOB
  2      DAC          PA6                 AIN6      AIN2      X2/Y2     AINN0     AINP1     AINP0     OUT
  3      LED          PA7                 AIN7      AIN3      X3/Y3     AINP0     AINP0     AINN0                                                                            LUT1-OUT
  4                   PB5       CLKOUT    AIN8                          AINP1               AINP2                                             *WO2
  5                   PB4                 AIN9                          AINN1     AINP3                                                       *WO1                          *LUT0-OUT
  6      RX           PB3       TOSC1                                             OUT                            RxD                          *WO0
  7      TX           PB2    TOSC2/EVOUT1                                                   OUT                  TxD                           WO2
  8      SDA          PB1                 AIN10               X4/Y4     AINP2                                    XCK                SDA        WO1
  9      SCL          PB0                 AIN11               X5/Y5               AINP2     AINP1                XDIR               SCL        WO0
  10                  PC0                           AIN6                                                                  *SCK                          *TCB0 WO  WOC
  11                  PC1                           AIN7                                                                  *MISO                                   WOD       *LUT1-OUT
  12                  PC2       EVOUT2              AIN8                                                                  *MOSI
  13                  PC3                           AIN9                                                                  *SS                 *WO3                           LUT1-IN0
  17     UPDI         PA0     RESET/UPDI  AIN0                                                                                                                               LUT0-IN0
  14     MOSI         PA1                 AIN1                                                                  *TxD       MOSI     *SDA                                     LUT0-IN1
  15     MISO         PA2       EVOUT0    AIN2                                                                  *RxD       MISO     *SCL                                     LUT0-IN2
  16     SCK          PA3       EXTCLK    AIN3                                                                  *XCK       SCK                 WO3       TCB1 WO
      * alternative pin locations
*/


namespace Pin
{
    // PA0 is UDPI
    const int8_t MOSI              = PIN_PA1;
    const int8_t MISO /*yum!*/     = PIN_PA2;
    const int8_t SCK               = PIN_PA3;
    const int8_t DISPL_DC          = PIN_PA4;
    const int8_t DISPL_CS          = PIN_PA5;
    const int8_t SOUND             = PIN_PA6; // DAC pin 
    const int8_t VBATT             = PIN_PA7;


    const int8_t POWER_SW          = PIN_PB0;
    const int8_t KCLK              = PIN_PB1;
    const int8_t TX                = PIN_PB2;
    const int8_t RX                = PIN_PB3;
    const int8_t KRST              = PIN_PB4;
    const int8_t CHG_SENSE         = PIN_PB5;

    const int8_t SOUND_EN          = PIN_PC0;
    const int8_t ISENSE            = PIN_PC1;
    const int8_t KBD0              = PIN_PC2;
    const int8_t KBD1              = PIN_PC3;

    const int8_t NONE               = -1;
}


extern Adafruit_SSD1306 _gfx;

extern const unsigned char font[];

#endif /* IO_H_ */