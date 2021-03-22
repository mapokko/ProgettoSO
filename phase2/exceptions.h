#include <pandos_types.h>
#include <pandos_const.h>
#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

void uTLB_RefillHandler ();
void kernelExcHandler();
void syscallHandler();

#endif