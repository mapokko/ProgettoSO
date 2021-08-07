#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#ifndef SYSCALL_H_INCLUDED
#define SYSCALL_H_INCLUDED

swap_t *sw_p;
int semSP;
int fifono;

 void initSwapStructs ();
 void Pager ();

#endif
