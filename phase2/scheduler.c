#include "scheduler.h"
#include "initial.h"
#include "exceptions.h"
#include "pcb.h"
#include "asl.h"
#include <pandos_types.h>
#include <pandos_const.h>

void scheduler(){

    static pcb_t *newCurrent;
    newCurrent = removeProcQ(&readyQ);
    if(readyQ == NULL){
        currentProcess = NULL;
    }

    if(newCurrent != NULL){
        currentProcess = newCurrent;

        
        // memaddr *timeScale;
        // timeScale = (memaddr) TIMESCALEADDR;
        // int timeLeft = TIMESLICE - (newCurrent->p_time % TIMESLICE);

        setTIMER(TIMESLICE);
        LDST(&(newCurrent->p_s));
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
