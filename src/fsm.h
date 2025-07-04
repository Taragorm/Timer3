#ifndef _FSM_H
#define _FSM_H


/**
 * Keyboard handling FSM
 */
extern char (*fsm)(char ch);
void resetFsm();
void clearPrompt();

#endif
