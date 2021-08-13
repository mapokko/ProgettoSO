#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#ifndef INTERRUPT_H_INCLUDED
#define INTERRUPT_H_INCLUDED

extern void interruptHandler();
extern memaddr *getDevReg(int devNumber, int interruptingDeviceNumber);


#endif