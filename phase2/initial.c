#include "initial.h"
#include "exceptions.h"
#include "scheduler.h"
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



void setIntervalTimer(){
    // memaddr *timeScale;
    // timeScale = (memaddr) TIMESCALEADDR;
    LDIT(100000);
}


void main(){
    static passupvector_t *passUpCP0;
    passUpCP0 = (memaddr) PASSUPVECTOR;
    passUpCP0->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    passUpCP0->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    passUpCP0->exception_handler = (memaddr) kernelExcHandler;
    passUpCP0->exception_stackPtr = (memaddr) KERNELSTACK;
    initPcbs();
    initASL();
    

    processCount = 0;
    blockedCount = 0;
    readyQ = mkEmptyProcQ();
    currentProcess = NULL;
    /* tecnicamente bisogna inizializzare tutti i semafori a 0
     * ma teoricamente, quando vengono dichiarati sono già inizializzati a 0
     * aggiungere eventualmente la loro inizializzazione qui.......
    */


    setIntervalTimer();

    static pcb_t *firstPcb;
    firstPcb = allocPcb();
    processCount++;
    firstPcb->p_s.status = IEPON | TEBITON | IMON;
    RAMTOP(firstPcb->p_s.reg_sp);
    firstPcb->p_s.pc_epc = firstPcb->p_s.reg_t9 = (memaddr) test;

    /* tecnicamente bisogna inizializzare tutti i campi alberi, semadd e support di firstPcb qui,
     * ma questo è già fatto da allocPcb, quindi dovrebbe già andare bene.....
    */


    insertProcQ(&readyQ, firstPcb);
    scheduler();
}

void memcpy(memaddr *src, memaddr *dest, unsigned int bytes){
    
    for(int i = 0; i < (bytes/4); i++){
        *dest = *src;
        dest++;
        src++;
    }
}

void STOP(){
    static int a;
    a = 0;
    SYSCALL(4, &(a), 0, 0);
    SYSCALL(3, &(a), 0, 0);
}

void cione(){
    while(1){;}
}

void trueSTOP(){;}