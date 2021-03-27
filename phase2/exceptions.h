#include <pandos_types.h>
#include <pandos_const.h>
#ifndef EXCEPTIONS_H_INCLUDED
#define EXCEPTIONS_H_INCLUDED

void uTLB_RefillHandler ();
void kernelExcHandler();
void syscallHandler();
void initDevSem();

pcb_t *verhogen(int *semAddrV);
memaddr *getDevReg(int devNumber, int bitMapLine);

#endif