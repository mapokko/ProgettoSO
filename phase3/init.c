#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
//#include "p2test.h"
#include "initProc.h"
#include <pandos_types.h>
#include <pandos_const.h>



void main(){

    initSystem();
    
    /*inizializzazione del primo pcb*/
    pcb_t *firstPcb;
    firstPcb = allocPcb();
    processCount++;
    firstPcb->p_s.status = IEPON | IMON;
    RAMTOP(firstPcb->p_s.reg_sp);
    firstPcb->p_s.pc_epc = firstPcb->p_s.reg_t9 = (memaddr) instantiatorProcess;

    /*inserimento del primo pcb nella readyQ e chiamata dello scheduler*/
    insertProcQ(&readyQ, firstPcb);
    scheduler();
}

/*inizializza passUp vector, strutture fase 1, variabili globali
 *e interrupt timer
*/
void initSystem(){
    /*inizializzazione passUp vector*/
    passupvector_t *passUpP0;
    passUpP0 = (memaddr) PASSUPVECTOR;
    passUpP0->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
    passUpP0->tlb_refill_stackPtr = (memaddr) KERNELSTACK;
    passUpP0->exception_handler = (memaddr) kernelExcHandler;
    passUpP0->exception_stackPtr = (memaddr) KERNELSTACK;

    /*inizializzazione strutture dati fase 1*/
    initPcbs();
    initASL();

    /*inizializzazione variabili globali*/
    processCount = 0;
    blockedCount = 0;
    readyQ = mkEmptyProcQ();
    currentProcess = NULL;
    bus_devReg_Area = RAMBASEADDR;

    /*interrupt timer a 100 millisecondi*/
    LDIT(PSECOND);

}
