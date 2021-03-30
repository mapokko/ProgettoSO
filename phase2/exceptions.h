#include <pandos_types.h>
#include <pandos_const.h>
#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

void kernelExcHandler();


memaddr *getDevReg(int devNumber, int bitMapLine);

state_t *currentState;

#endif