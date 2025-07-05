#include "timers.h"
#include <Arduino.h>

Timer _timers[TIMERCOUNT];

//--------------------------------------------------
void Timer::rewind()
{
    secs = 0;
    mins = timer.mins;
    times = timer.times;
}
//--------------------------------------------------
void Timer::start()
{
    if(isDone())
        rewind();
    running = isConfigured();
}
//--------------------------------------------------
void Timer::stop()
{
    running = false;
}
//--------------------------------------------------
bool Timer::isDone() const
{
    return times==0 && mins==0 && secs==0;
}
//--------------------------------------------------
bool Timer::tick()
{
    if(isDone() || !running)
        return false;

    bool ret = false;

    if(secs==0)
    {
        if(mins==0)
        {
            // not the first time
            ret = true;
            --times; 
            if(times>0)
            {
                mins = timer.mins-1;
                secs = 59;
            }
            else
            {
                running = false;
            }
        }
        else
        {
            secs = 59;
            --mins;
        }
    }
    else
        --secs;

    return ret;
}
//--------------------------------------------------
void Timer::clear()
{
    timer.mins = 0;
    timer.times = 0;
    secs = 0;
    mins = 0;
    times = 0;
    running = false;
}
//--------------------------------------------------
bool Timer::isConfigured() const
{
    return timer.mins!=0 && timer.times!=0;
}
//--------------------------------------------------
char Timer::stateChar() const
{
    if(isDone())
        return 0x80;

    if(running)
        return 0x82;
    else
        return 0x81;
    
}
//--------------------------------------------------

