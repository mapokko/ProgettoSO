#include "pandos_const.h"
#include "pandos_types.h"
#include "sysSupport.h"
#include <umps3/umps/libumps.h>

swap_t swapPoolTable[POOLSIZE];
int swapPoolSem;
static int fifoNo = 0;

void Pager(){
    support_t *sPtr = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *exState = &(sPtr->sup_exceptState[PGFAULTEXCEPT]);
    unsigned int exCause = (exState->cause & GETEXECCODE) >> 2;

    if(exCause == 1 ){

    }

    SYSCALL(PASSEREN, &(swapPoolSem), 0, 0);

    int excPageNo = (exState->entry_hi & GETPAGENO) >> 12;
    swap_t *framePtr = pagingFIFO();

    if(framePtr->sw_asid != -1){
        framePtr->sw_pte->pte_entryLO |= !(VALIDON);
        updateTLB(framePtr->sw_pte);

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

swap_t* pagingFIFO(){
    //verificare ordine di valutazione di fifoNo++
    return &(swapPoolTable[fifoNo++ % POOLSIZE]);
}

void updateTLB(pteEntry_t *entry){

    setENTRYHI(entry->pte_entryHI);
    TLBP();

    if(getINDEX() & PRESENTFLAG){
        setENTRYHI(entry->pte_entryHI);
        setENTRYLO(entry->pte_entryLO);
        TLBWI();
    }
}