#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#ifndef SYSSUPPORT_H_INCLUDED
#define SYSSUPPORT_H_INCLUDED

extern void generalSupHandler();
extern void trapHandler(int supSem, support_t *sPtr);


#endif