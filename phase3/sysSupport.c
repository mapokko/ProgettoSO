#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>


void generalSupHandler(){
    support_t *sPtr = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    static unsigned int genExCause;
    genExCause = (sPtr->sup_exceptState[GENERALEXCEPT].cause & GETEXECCODE) >> 2;
    static unsigned int a0Val;
    a0Val = sPtr->sup_exceptState[GENERALEXCEPT].reg_a0;

    if(genExCause == 8){
        supportSyscallHandler(&(sPtr->sup_exceptState[GENERALEXCEPT]));
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