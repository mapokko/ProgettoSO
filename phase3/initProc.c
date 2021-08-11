#include "pandos_const.h"
#include "pandos_types.h"
#include "vmSupport.h"
#include "sysSupport.h"
#include <umps3/umps/libumps.h>

static support_t supportTable[UPROCMAX];

static int privsem = 0;

void initUPageTable(int ASID){
    for(int i = 0; i < USERPGTBLSIZE - 1; i++){
        supportTable[ASID].sup_privatePgTbl[i].pte_entryHI = ((0x80000 | i ) << VPNSHIFT) | ((ASID + 1) << ASIDSHIFT);
        supportTable[ASID].sup_privatePgTbl[i].pte_entryLO = DIRTYON;
    }
    static unsigned int stackAddr;
    stackAddr = (0xBFFFF << VPNSHIFT) | ((ASID + 1) << ASIDSHIFT);
    FERMATI3();
    supportTable[ASID].sup_privatePgTbl[31].pte_entryHI = (0xBFFFF << VPNSHIFT) | ((ASID + 1) << ASIDSHIFT);
    supportTable[ASID].sup_privatePgTbl[31].pte_entryLO = DIRTYON;

}

void initUProc(){
    for(int i = 0; i < 2; i++){
        state_t newState;
        newState.pc_epc = newState.reg_t9 = UPROCSTARTADDR;
        newState.reg_sp = USERSTACKTOP;
        newState.status = TEBITON | IMON | IEPON | USERPON;
        newState.entry_hi = (i + 1) << ASIDSHIFT;
        
        supportTable[i].sup_asid = i + 1;
        supportTable[i].sup_exceptContext[PGFAULTEXCEPT].c_pc = (memaddr *) Pager;
        supportTable[i].sup_exceptContext[GENERALEXCEPT].c_pc = (memaddr *) generalSupHandler;
        supportTable[i].sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = &(supportTable[i].sup_stackTLB[499]);
        supportTable[i].sup_exceptContext[GENERALEXCEPT].c_stackPtr = &(supportTable[i].sup_stackGen[499]);
        supportTable[i].sup_exceptContext[PGFAULTEXCEPT].c_status = supportTable[i].sup_exceptContext[GENERALEXCEPT].c_status = IMON | IEPON;

        initUPageTable(i);

        SYSCALL(CREATEPROCESS, (memaddr) &(newState), (memaddr) &(supportTable[i]), 0);
    }
}

void instantiatorProcess(){

    initSwapPool();

    initSupSem();

    
    initUProc();
    
    SYSCALL(PASSEREN, &(privsem), 0, 0);
}

void FERMATI(){}

void FERMATI3(){}