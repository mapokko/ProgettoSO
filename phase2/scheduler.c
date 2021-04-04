#include "scheduler.h"
#include "initial.h"
#include "exceptions.h"
#include "pcb.h"
#include "asl.h"
#include <pandos_types.h>
#include <pandos_const.h>



void scheduler(){
    currentProcess = removeProcQ(&readyQ);

    if(currentProcess != NULL){
        STCK(processStartTime);
    
        setTIMER(TIMESLICE);
        LDST(&(currentProcess->p_s));
    }
    else if(processCount == 0){
        HALT();
    }
    else if(blockedCount == 0){
        PANIC();
    }
    else{
        setSTATUS(IECON | IMON);
        WAIT();
    }
}
