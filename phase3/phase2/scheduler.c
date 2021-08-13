#include "scheduler.h"
#include "init.h"
#include "phase1/pcb.h"
#include "phase1/asl.h"
#include <pandos_types.h>
#include <pandos_const.h>
#include <umps3/umps/libumps.h>



void scheduler(){
    /*rimoszione dalla readyQ del primo processo pronto*/
    currentProcess = removeProcQ(&readyQ);

    /*se l'estrazione è OK, si salva il TOD attuale
     *si carica il PLT con 5 millisecondi e
     *si cede il controllo al processo estratto
    */
    if(currentProcess != NULL){
        STCK(processStartTime);
        
        setTIMER(TIMESLICE * (*((cpu_t *) TIMESCALEADDR)));
        LDST(&(currentProcess->p_s));
    }
    /*se l'estrazione non è OK, accade uno di queste*/

    /*se non ci sono processi nel sistema*/
    else if(processCount == 0){
        HALT();
    }
    /*se c'è un deadlock*/
    else if(blockedCount == 0){
        PANIC();
    }
    /*attesa di un interrupt*/
    else{
        setSTATUS(IECON | IMON);
        WAIT();
    }
}
