#ifndef _ANALOGS_H
#define _ANALOGS_H
#include <stdint.h>

void analogInit();
bool readAnalogs(bool first);

extern uint16_t _chargemA;
extern uint16_t _battmv;
extern uint16_t _battPercent;


#endif