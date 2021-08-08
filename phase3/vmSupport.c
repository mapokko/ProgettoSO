#include "pandos_const.h"
#include "pandos_types.h"
#include "sysSupport.h"
#include <umps3/umps/libumps.h>

swap_t swapPoolTable[POOLSIZE];
int swapPoolSem;

void Pager(){
    support_t *sPtr = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    unsigned int exCause = (sPtr->sup_exceptState[PGFAULTEXCEPT].cause & GETEXECCODE) >> 2;

    if(exCause == 1){

    }



}

memaddr swapPoolBegin(){
    return *((memaddr *)(KERNELSTACK + 0x0018)) + *((memaddr *)(KERNELSTACK + 0x0024));
}

void initSwapPool(){
    
    swapPoolSem = 1;

    for(int i = 0; i < POOLSIZE; i++){
        swapPoolTable[i].sw_asid = -1;
        swapPoolTable[i].sw_pageNo = 0;
        swapPoolTable[i].sw_pte = NULL;
    }
}