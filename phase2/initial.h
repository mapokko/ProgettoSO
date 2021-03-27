#include <pandos_types.h>
#include <pandos_const.h>
#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED


pcb_t *readyQ;
pcb_t *currentProcess;

unsigned int processCount;
unsigned int blockedCount;
int devSem[4][8];
int terSem[2][8];
int pseudoClock;

void memcpy(memaddr *src, memaddr *dest, unsigned int words);
void STOP();
void cione();
void trueSTOP();

#endif