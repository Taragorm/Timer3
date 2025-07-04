/**
 * @file Sound support, using TMRB0
 */
#ifndef _SOUND_H
#define _SOUND_H
#include <stdint.h>

void soundInit();
void soundSleep();
void soundLoop();
void setFreq(uint16_t f);

 #endif