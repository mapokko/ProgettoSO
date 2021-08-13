#include <pandos_types.h>
#include <pandos_const.h>
#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

void kernelExcHandler();
void memcpy(memaddr *src, memaddr *dest, unsigned int words);

state_t *currentState;

#endif