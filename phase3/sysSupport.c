#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>


void generalSupHandler(){
    support_t *sPtr = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    unsigned int exCause = (sPtr->sup_exceptState[PGFAULTEXCEPT].cause & GETEXECCODE) >> 2;

    if(exCause == 8){
        supportSyscallHandler(&(sPtr->sup_exceptState[PGFAULTEXCEPT]));
    }
    else{
        
    }
}

void supportSyscallHandler(state_t *suppState){
    suppState->pc_epc += 4;

    switch(suppState->reg_a0){
        case TERMINATE:
        terminate();
        break;
        case GET_TOD:
        getTOD(suppState);
        break;
        case WRITEPRINTER:

        break;
        case WRITETERMINAL:

        break;
        case READTERMINAL:

        break;
        default:

        break;
    }
}

void terminate(){
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

void getTOD(state_t *suppState){
    STCK(suppState->reg_v0);
    LDST(suppState); /*serve????*/
}