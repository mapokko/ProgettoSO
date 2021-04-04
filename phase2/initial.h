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
unsigned int processStartTime;
devregarea_t *bus_devReg_Area;


void trueSTOP();

#endif