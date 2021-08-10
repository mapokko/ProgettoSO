#include "pandos_const.h"
#include "pandos_types.h"
#include "sysSupport.h"
#include "interrupt.h"
#include "initProc.h"
#include <umps3/umps/libumps.h>
extern devSem[4][8];
swap_t swapPoolTable[POOLSIZE];
static memaddr swapPoolPtr;
int swapPoolSem;
static int fifoNo = -1;
swap_t* pagingFIFO(memaddr *frame);

/*  i semafori di supporto sono così strutturati: (SOGGETTO AD EVENTUALE COMBIAMENTO)
    *i semafori supDevSem[0][0...7] definiscono i device flash (in ordine)
    *i semafori supDevSem[1][0...7] definiscono i printer (in ordine) 
    *i semafori supDevSem[2][0...7] definiscono i terminali in scrittura (in ordine)
    *i semafori supDevSem[3][0...7] definiscono i terminali in lettura (in ordine)*/
int supDevSem[4][8];

void Pager(){
    support_t *sPtr = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *exState = &(sPtr->sup_exceptState[PGFAULTEXCEPT]);
    static unsigned int exCause;
    exCause = (exState->cause & GETEXECCODE) >> 2;

    if(exCause == 1 ){

    }

    SYSCALL(PASSEREN, &(swapPoolSem), 0, 0);

    static int excPageNo;

    excPageNo = (exState->entry_hi & GETPAGENO) >> VPNSHIFT;
    if(excPageNo == 0x3FFFF){
        excPageNo = 31;
    }
    static memaddr frameAddr;
    static swap_t *poolTablePtr;
    poolTablePtr = pagingFIFO(&(frameAddr));

    if(poolTablePtr->sw_asid != -1){

        setSTATUS(getSTATUS() & ~IECON);

        poolTablePtr->sw_pte->pte_entryLO &= ~VALIDON;
        
        setENTRYHI(poolTablePtr->sw_pte->pte_entryHI);
        TLBP();
        if(getINDEX() & PRESENTFLAG){
            setENTRYLO(poolTablePtr->sw_pte->pte_entryLO);
            TLBWI();
        }

        setSTATUS(getSTATUS() | IECON);
        
        static int blockToUpload, PFNtoUpload;

        blockToUpload = (poolTablePtr->sw_pte->pte_entryHI & GETPAGENO) >> VPNSHIFT;
        if(blockToUpload == 0x3FFFF){
            blockToUpload = 31;
        }

        PFNtoUpload = poolTablePtr->sw_pte->pte_entryLO & 0xFFFFF000;

        RWflash(blockToUpload, PFNtoUpload, poolTablePtr->sw_asid - 1, FLASHWRITE);

    }
    
    RWflash(excPageNo, frameAddr, sPtr->sup_asid - 1, FLASHREAD);

    poolTablePtr->sw_asid = sPtr->sup_asid;
    poolTablePtr->sw_pageNo = excPageNo;
    poolTablePtr->sw_pte = &(sPtr->sup_privatePgTbl[excPageNo]);

    setSTATUS(getSTATUS() & ~IECON);
    
    sPtr->sup_privatePgTbl[excPageNo].pte_entryLO = frameAddr | VALIDON | DIRTYON;
    FERMATI();
    updateTLB(&(sPtr->sup_privatePgTbl[excPageNo]));

    setSTATUS(getSTATUS() | IECON);

    SYSCALL(VERHOGEN, &(swapPoolSem), 0, 0);
    FERMATI();
    LDST(exState);

}

memaddr swapPoolBegin(){
    return *((memaddr *)(KERNELSTACK + 0x0018)) + *((memaddr *)(KERNELSTACK + 0x0024));
}

void initSupSem(){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 8; j++){
            supDevSem[i][j] = 1;
        }
    }
}

void initSwapPool(){
    
    swapPoolSem = 1;
    swapPoolPtr = swapPoolBegin();

    for(int i = 0; i < POOLSIZE; i++){
        swapPoolTable[i].sw_asid = -1;
        swapPoolTable[i].sw_pageNo = 0;
        swapPoolTable[i].sw_pte = NULL;
    }
}

swap_t* pagingFIFO(memaddr *frame){
    fifoNo++;
    
    *frame = swapPoolPtr + (0x1000 * (fifoNo % POOLSIZE));
    return &(swapPoolTable[fifoNo % POOLSIZE]);
}

void updateTLB(pteEntry_t *entry){

    setENTRYHI(entry->pte_entryHI);
    TLBP();
    setENTRYHI(entry->pte_entryHI);
    setENTRYLO(entry->pte_entryLO);

    if(!(getINDEX() & PRESENTFLAG)){
        TLBWI();
    }
    else{
        TLBWR();
    }
}

void RWflash(int blockNo, memaddr ramAddr, int flashDevNo, int flag){

    SYSCALL(PASSEREN, &(supDevSem[0][flashDevNo]), 0, 0);

    dtpreg_t *devReg;
    devReg = getDevReg(4, flashDevNo);
    devReg->data0 = ramAddr;

    setSTATUS(getSTATUS() & ~IECON);
    devReg->command = (blockNo << 8) | flag;
    
    SYSCALL(IOWAIT, FLASHINT, flashDevNo, 0);
    
    setSTATUS(getSTATUS() | IECON);

    if(devReg->status != 1){
        /*chiamare gestore dei program trap*/
    }

    SYSCALL(VERHOGEN, &(supDevSem[0][flashDevNo]), 0, 0);
}

