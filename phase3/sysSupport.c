#include "pandos_const.h"
#include "pandos_types.h"
#include "interrupt.h"
#include "vmSupport.h"
#include <umps3/umps/libumps.h>

//support_t *sPtr;

void generalSupHandler(){
    support_t *sPtr = SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    unsigned int genExCause;
    genExCause = (sPtr->sup_exceptState[GENERALEXCEPT].cause & GETEXECCODE) >> 2;
    

    if(genExCause == 8){
        supportSyscallHandler(&(sPtr->sup_exceptState[GENERALEXCEPT]), sPtr);
    }
    else{
        trapHandler(sPtr->sup_asid - 1);
    }
}

void supportSyscallHandler(state_t *genState, support_t *sPtr){
    genState->pc_epc += 4;
    switch(genState->reg_a0){
        case TERMINATE:
        terminate();
        break;
        case GET_TOD:
        getTOD(genState);
        break;
        case WRITEPRINTER:
        Write_To_Printer(genState->reg_a1, genState->reg_a2, sPtr);
        break;
        case WRITETERMINAL:
        Write_To_Terminal(genState->reg_a1, genState->reg_a2, sPtr);
        break;
        case READTERMINAL:
        Read_From_terminal(genState->reg_a1, sPtr);
        break;
        default:
        trapHandler(sPtr->sup_asid - 1);
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

void Write_To_Printer (char *string, int len, support_t *sPtr) {
     if(len>128 || len<1 || string < 0x80000000 || string > 0xC0000000 ){

        SYSCALL(TERMPROCESS, 0, 0, 0);
    }

    dtpreg_t *printReg = getDevReg(PRNTINT, sPtr->sup_asid - 1);
    int opStatusP = 0;
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
        
    }

    if ( opStatusP == 1) {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = len;

    } else {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = ~opStatusP;
    }

    SYSCALL(VERHOGEN, &(supDevSem[1][sPtr->sup_asid - 1]), 0, 0);
    LDST(&(sPtr->sup_exceptState[GENERALEXCEPT]));
}

void Write_To_Terminal(char *string, int len, support_t *sPtr){

    if(len < 0 || len > 128 || string < 0x80000000 || string > 0xC0000000){
        terminate();
    }

    termreg_t *termReg;
    termReg = getDevReg(TERMINT, sPtr->sup_asid - 1);
    int opStatus;

    SYSCALL(PASSEREN, &(supDevSem[2][sPtr->sup_asid - 1]), 0, 0);
    
    while(*string != EOS){
        FERMATIsys();
        setSTATUS(getSTATUS() & ~IECON);
        termReg->transm_command = 2 | ((*string) << 8);
        opStatus = SYSCALL(IOWAIT, TERMINT, sPtr->sup_asid - 1, 0);
        setSTATUS(getSTATUS() | IECON);
        
        if((opStatus & 0xFF) != 5){
            break;
        }
        string++;
        FERMATIsys();
    }
    
    if ( (opStatus & 0xFF) == 5) {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = len;

    } else {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = ~opStatus;
    }
    SYSCALL(VERHOGEN, &(supDevSem[2][sPtr->sup_asid - 1]), 0, 0);
  
    LDST(&(sPtr->sup_exceptState[GENERALEXCEPT]));
}

void Read_From_terminal(char *stringAddr, support_t *sPtr){
    if(stringAddr < 0x80000000 || stringAddr > 0xC0000000){
        terminate();
    }

    termreg_t *termRegRead;
    termRegRead = getDevReg(TERMINT, sPtr->sup_asid - 1);
    int opStatusRead, count = 0;
    char receivedChar = NULL;
    
    SYSCALL(PASSEREN, &(supDevSem[3][sPtr->sup_asid - 1]), 0, 0);
    
    
    while(receivedChar != '\n'){

        setSTATUS(getSTATUS() & ~IECON);
        termRegRead->recv_command = 2;
        opStatusRead = SYSCALL(IOWAIT, TERMINT, sPtr->sup_asid - 1, 1);
        setSTATUS(getSTATUS() | IECON);

        receivedChar = (opStatusRead & 0xFF00) >> 8;
        *stringAddr = receivedChar;
        if((opStatusRead & 0xFF) != 5){
            break;
        }
        count++;
        stringAddr++;
        
    }

    if ( (opStatusRead & 0xFF) == 5) {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = count;

    } else {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = ~opStatusRead;
    }


    SYSCALL(VERHOGEN, &(supDevSem[3][sPtr->sup_asid - 1]), 0, 0);
    LDST(&(sPtr->sup_exceptState[GENERALEXCEPT]));

}

void trapHandler(int supSem){
    for(int i = 0; i < 4; i++){
        if(!(supDevSem[i][supSem])){
            SYSCALL(VERHOGEN, &(supDevSem[i][supSem]), 0, 0);
        }   
    }
    terminate();
}

void FERMATIsys(){}
