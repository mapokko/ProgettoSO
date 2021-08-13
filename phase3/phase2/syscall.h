#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

void syscallHandler();
pcb_t *verhogen(int *semAddrV);
void updateCPUtime();
void passUp_orDie(int contextPosition);

#endif