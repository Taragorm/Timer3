#ifndef _TIMERS_H
#define _TIMERS_H

#include <stdint.h>

const int TIMERCOUNT=6;

//-------------------------------------------
struct TimerInfo
{
    uint16_t mins;
    uint16_t times;
};
//-------------------------------------------
struct Timer
{
    // settings configured
    TimerInfo timer;

    // elasped
    uint16_t mins;
    uint16_t secs;
    uint16_t times;
    bool running;

    /// @brief Clear down the timer
    void clear();

    /// @brief Have we been configured
    /// @return 
    bool isConfigured() const;

    /// @brief reset the elapsed time to the beginning of the interval
    void rewind();

    /// @brief start the timer (if possible)
    void start();

    /// @brief Stop the timer
    void stop();

    /// @brief Call once per second to run the timers
    /// @return \a true when the timer has elapsed
    bool tick();

    /// @brief Are we done?
    /// @return 
    bool isDone() const;
};
//-------------------------------------------

typedef  void (Timer::*TimerMemFn)(void);  


extern Timer _timers[TIMERCOUNT];

#endif