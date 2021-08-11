#include "pandos_const.h"
#include "pandos_types.h"
#include "interrupt.h"
#include "vmSupport.h"
#include <umps3/umps/libumps.h>

support_t *sPtr;

void generalSupHandler(){
    sPtr = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    static unsigned int genExCause;
    genExCause = (sPtr->sup_exceptState[GENERALEXCEPT].cause & GETEXECCODE) >> 2;
    static unsigned int a0Val;
    a0Val = sPtr->sup_asid;

    if(genExCause == 8){
        supportSyscallHandler(&(sPtr->sup_exceptState[GENERALEXCEPT]));
    }
    else{

    }
}

void supportSyscallHandler(state_t *genState){
    genState->pc_epc += 4;
    switch(genState->reg_v0){
        case TERMINATE:
        terminate();
        break;
        case GET_TOD:
        getTOD(genState);
        break;
        case WRITEPRINTER:
        Write_To_Printer(genState->reg_a1, genState->reg_a2);
        break;
        case WRITETERMINAL:
        Write_To_Terminal(genState->reg_a1, genState->reg_a2);
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

void Write_To_Printer (char *string, int len) {
     if(len>128 || len<1 || string < 0x80000000 || string > 0xC0000000 ){

        SYSCALL(TERMPROCESS, 0, 0, 0);
    }

    dtpreg_t *printReg = getDevReg(PRNTINT, sPtr->sup_asid - 1);
    static int opStatusP = 0;
    SYSCALL(PASSEREN, &(supDevSem[1][sPtr->sup_asid - 1]), 0, 0);

    while(*string != EOS){
        printReg->data0 = *string;

        setSTATUS(getSTATUS() & ~IECON);
        printReg->command = 2;
        opStatusP = SYSCALL(IOWAIT, PRNTINT, sPtr->sup_asid - 1, 0);
        setSTATUS(getSTATUS() | IECON);

        if(opStatusP != 1){
            break;
        }
        string++;
        FERMATIsys();
    }

    if ( opStatusP == 1) {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = len;

    } else {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = ~opStatusP;
    }

    SYSCALL(VERHOGEN, &(supDevSem[1][sPtr->sup_asid - 1]), 0, 0);
    LDST(&(sPtr->sup_exceptState[GENERALEXCEPT]));
}

void Write_To_Terminal(char *string, int len){

    if(len < 0 || len > 128 || string < 0x80000000 || string > 0xC0000000){
        terminate();
    }

    static termreg_t *termReg;
    termReg = getDevReg(TERMINT, sPtr->sup_asid - 1);
    int opStatus;

    SYSCALL(PASSEREN, &(supDevSem[2][sPtr->sup_asid - 1]), 0, 0);
    while(*string != EOS){

        setSTATUS(getSTATUS() & ~IECON);
        termReg->transm_command = 2 | ((*string) << 8);
        opStatus = SYSCALL(IOWAIT, TERMINT, sPtr->sup_asid - 1, 0);
        setSTATUS(getSTATUS() & ~IECON);
        
        if((opStatus & 0xFF) != 5){
            break;
        }
        string++;
    }

    if ( (opStatus & 0xFF) == 5) {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = len;

    } else {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = ~opStatus;
    }
    SYSCALL(VERHOGEN, &(supDevSem[2][sPtr->sup_asid - 1]), 0, 0);
    FERMATIsys();
    LDST(&(sPtr->sup_exceptState[GENERALEXCEPT]));
}

void FERMATIsys(){}
