#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#ifndef VMSUPPORT_H_INCLUDED
#define VMSUPPORT_H_INCLUDED

extern swap_t swapPoolTable[POOLSIZE];
extern int swapPoolSem;
void initSwapPool();

void Pager();
void initSupSem();


#endif