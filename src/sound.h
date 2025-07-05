/**
 * @file Sound support, using TMRB0
 */
#ifndef _SOUND_H
#define _SOUND_H
#include <stdint.h>

void soundStaticInit();
void soundInit();
void soundSleep();
void soundLoop();
void setFreq(uint16_t f);
void playSound(int index);
bool isPlayingSound();

const int MAXSOUND = 10;

//-----------------------------------------------
class ISound
{
protected:    
    ISound* _next;

    ISound(ISound* n)
    : _next(n)
    {}

public:

    ISound* next() const  { return _next; }
    virtual ISound* execute() = 0;    
    virtual void init(bool first) = 0;
    ISound* makeCircular();
};
//-----------------------------------------------


 #endif