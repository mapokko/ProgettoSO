#include "initial.h"
#include "exceptions.h"
#include "scheduler.h"
#include "syscall.h"
#include "interrupt.h"
#include "pcb.h"
#include "asl.h"
#include "p2test.h"
#include <pandos_types.h>
#include <pandos_const.h>
//#include "/usr/include/umps3/umps/libumps.h"


unsigned int processCount;
unsigned int blockedCount;
int devSem[4][8];
int terSem[2][8];
int pseudoClock;
unsigned int processStartTime;
devregarea_t *bus_devReg_Area = RAMBASEADDR;


void main(){
    initSystem();
    

    
    static pcb_t *firstPcb;
    firstPcb = allocPcb();
    processCount++;
    firstPcb->p_s.status = IEPON | TEBITON | IMON;
    RAMTOP(firstPcb->p_s.reg_sp);
    firstPcb->p_s.pc_epc = firstPcb->p_s.reg_t9 = (memaddr) test;


    insertProcQ(&readyQ, firstPcb);
    scheduler();
}

void initSystem(){
    static passupvector_t *passUpP0;
    passUpP0 = (memaddr) PASSUPVECTOR;
    passUpP0->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    passUpP0->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    passUpP0->exception_handler = (memaddr) kernelExcHandler;
    passUpP0->exception_stackPtr = (memaddr) KERNELSTACK;

    initPcbs();
    initASL();

    processCount = 0;
    blockedCount = 0;
    readyQ = mkEmptyProcQ();
    currentProcess = NULL;

    LDIT(PSECOND);

}



void trueSTOP(){;}