#include "pandos_const.h"
#include "pandos_types.h"
#include "phase2/interrupt.h"
#include "vmSupport.h"
#include "initProc.h"
#include <umps3/umps/libumps.h>

/*  dichiarazioni delle funzioni nel file corrente*/
void supportSyscallHandler(state_t *genState, support_t *sPtr);
void terminate(support_t *sPtr);
void getTOD(state_t *suppState);
void Write_To_Printer (char *string, int len, support_t *sPtr);
void Write_To_Terminal(char *string, int len, support_t *sPtr);
void Read_From_terminal(char *stringAddr, support_t *sPtr);
void trapHandler(int supSem, support_t *sPtr);

/*  gestore generale delle funzioni di support, smista tra SYSCALL > 8 e trap handler*/
void generalSupHandler(){

    /*  si ottiene la struttura di supporto e si estrae la causa*/
    support_t *sPtr = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    unsigned int genExCause;
    genExCause = (sPtr->sup_exceptState[GENERALEXCEPT].cause & GETEXECCODE) >> 2;
    
    /*  se la causa è una SYSCALL, si continua con il gestore delle SYSCALL di supporto
        altrimenti è program trap*/
    if(genExCause == 8){
        supportSyscallHandler(&(sPtr->sup_exceptState[GENERALEXCEPT]), sPtr);
    }
    else{
        trapHandler(sPtr->sup_asid - 1, sPtr);
    }
}

/*  gestore delle SYSCALL di supporto*/
void supportSyscallHandler(state_t *genState, support_t *sPtr){

    /*  si incrementa il program counter all'indirizzo successivo alla chiamata della 
        SYSCALL per riprendere correttamente l'esecuzione con un eventuale LDST*/
    genState->pc_epc += 4;

    /*  si smista tra la SYSCALL corretta, se SYSCALL > 13, si chiama trapHandler*/
    switch(genState->reg_a0){
        case TERMINATE:
        terminate(sPtr);
        break;
        case GET_TOD:
        getTOD(genState);
        break;
        case WRITEPRINTER:
        Write_To_Printer((char *)genState->reg_a1, genState->reg_a2, sPtr);
        break;
        case WRITETERMINAL:
        Write_To_Terminal((char *)genState->reg_a1, genState->reg_a2, sPtr);
        break;
        case READTERMINAL:
        Read_From_terminal((char *)genState->reg_a1, sPtr);
        break;
        default:
        trapHandler(sPtr->sup_asid - 1, sPtr);
        break;
    }
}

/*  SYSCALL 9: wrapper per SYSCALL 2*/
void terminate(support_t *sPtr){
    /*  prima di terminare il processo, invalita tutti i frame della swap pool
        impiegati dal processo corrente*/
    for(int i = 0; i < POOLSIZE; i++){
        if(sPtr->sup_asid == swapPoolTable[i].sw_asid){
            swapPoolTable[i].sw_asid = -1;
        }
    }

    /*  viene fatto un SYS4 per sbloccare lo instantiatorProcess*/
    SYSCALL(VERHOGEN, (int)&(masterSem), 0, 0);

    /*  si elimina il processo chiamando SYS2*/
    SYSCALL(TERMPROCESS, 0, 0, 0);
}

/*  SYSCALL 10: scrive nel registro di ritorno il TOD*/
void getTOD(state_t *suppState){
    STCK(suppState->reg_v0);
    LDST(suppState);
}

/*  SYSCALL 11: stampa nel Printer*/
void Write_To_Printer (char *string, int len, support_t *sPtr) {

    /*  si verifica che la stringa sia entro i limiti di lunghezza e l'indirizzo di
        memoria virtuale da cui si legge il carattere rientri nell'address space del U-proc*/
    if(len > 128 || len < 1 || (int)string < 0x80000000 || (int)string > 0xC0000000 ){
        terminate(sPtr);
    }

    /*  si estrae il registro del printer associato al U-proc*/
    dtpreg_t *printReg = (dtpreg_t *)getDevReg(PRNTINT, sPtr->sup_asid - 1);
    int opStatusP;

    /*  si richiede l'accesso al semaforo di support level per il printer associato*/
    SYSCALL(PASSEREN, (int)&(supDevSem[1][sPtr->sup_asid - 1]), 0, 0);

    /*  ciclo che stampa un carattere alla volta nel printer*/
    while(*string != EOS){

        /*  imposta il campo DATA0 del registro del Printer con un carattere della stringa*/
        printReg->data0 = *string;

        /*  si disabilitano gli interrupt per evitare che arrivi prima della SYS5
            si da poi il comando usando il campo COMMAND del printer e si blocca il processo
            al corretto semaforo di interrupt per il device*/
        setSTATUS(getSTATUS() & ~IECON);
        printReg->command = 2;
        opStatusP = SYSCALL(IOWAIT, PRNTINT, sPtr->sup_asid - 1, 0);
        setSTATUS(getSTATUS() | IECON);

        /*  ripresa l'esecuzione, si verifica se l'operazione è andata a buon fine.
            in caso negativo, si esce dal ciclo*/
        if(opStatusP != 1){
            break;
        }

        /*  si avanza di un carattere*/
        string++;
        
    }

    /*  se la trasmissione della stringa ha avuto successo fino all'ultimo
        carattere, si comunica nel registro di ritorno la lunghezza della stringa trasmessa
        altrimento il negato dello stato di errore*/
    if ( opStatusP == 1) {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = len;

    } else {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = ~opStatusP;
    }

    /*  si libera l'accesso al deviec printer*/
    SYSCALL(VERHOGEN, (int)&(supDevSem[1][sPtr->sup_asid - 1]), 0, 0);

    /*  si riprende l'esecuzione del U-proc*/
    LDST(&(sPtr->sup_exceptState[GENERALEXCEPT]));
}

/*  SYSCALL 12: stampa su terminale la stringa passata*/
void Write_To_Terminal(char *string, int len, support_t *sPtr){

    /*  si verifica che la stringa sia entro i limiti di lunghezza e l'indirizzo di
        memoria virtuale da cui si legge il carattere rientri nell'address space del U-proc*/
    if(len > 128 || len < 1 || (int)string < 0x80000000 || (int)string > 0xC0000000 ){
        terminate(sPtr);
    }

    /*  si individua il registro del terminale associato al U-proc*/
    termreg_t *termReg = (termreg_t *)getDevReg(TERMINT, sPtr->sup_asid - 1);
    int opStatus;

    /*  si blocca l'accesso al semaforo del terminale in scrittura*/
    SYSCALL(PASSEREN, (int)&(supDevSem[2][sPtr->sup_asid - 1]), 0, 0);
    
    /*  ciclo che stampa un carattere alla volta nel terminale*/
    while(*string != EOS){

        /*  si disabilitano gli interrupt per evitare che arrivi prima della SYS5
            si da poi il comando usando il campo COMMAND del terminale in scrittura, 
            impostando anche il carattere da trasmettere e si blocca il processo
            al corretto semaforo di interrupt per il device*/
        setSTATUS(getSTATUS() & ~IECON);
        termReg->transm_command = 2 | ((*string) << 8);
        opStatus = SYSCALL(IOWAIT, TERMINT, sPtr->sup_asid - 1, 0);
        setSTATUS(getSTATUS() | IECON);
        
        /*  se l'operazione non è andata a buon fine si esce dal ciclo*/
        if((opStatus & 0xFF) != 5){
            break;
        }

        /*  si procede al char successivo*/
        string++;
    }
    
    /*  se l'intera trasmissione ha avuto successo, si comunica
        la lunghezza delal stringa, altrimenti il negato dello stato di errore*/
    if ( (opStatus & 0xFF) == 5) {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = len;

    } else {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = ~opStatus;
    }

    /*  si rilascia l'accesso al terminale in scrittura*/
    SYSCALL(VERHOGEN, (int)&(supDevSem[2][sPtr->sup_asid - 1]), 0, 0);
  
    /*  si riprende l'esecuzione del U-proc*/
    LDST(&(sPtr->sup_exceptState[GENERALEXCEPT]));
}

/*  SYSCALL 13: legge dal terminale*/
void Read_From_terminal(char *stringAddr, support_t *sPtr){

    /*  si verifica che l'indirizzo di memoria virtuale dove si deve scrivere la stringa 
        rientri nell'address space del U-proc*/
    if((int)stringAddr < 0x80000000 || (int)stringAddr > 0xC0000000){
        terminate(sPtr);
    }

    /*  si individua il registro del terminale associato al U-proc, si mantiene inoltre
        un intero "count" che indica il numero di caratteri letti*/
    termreg_t *termRegRead = (termreg_t *)getDevReg(TERMINT, sPtr->sup_asid - 1);
    int opStatusRead, count = 0;
    char receivedChar;
    
    /*  si richiede l'accesso al terminale in lettura*/
    SYSCALL(PASSEREN, (int)&(supDevSem[3][sPtr->sup_asid - 1]), 0, 0);
    
    /*  si leggono i caratteri del termianel fino ad una newline*/
    while(receivedChar != '\n'){

        /*  si disabilitano gli interrupt per evitare che arrivi prima della SYS5
            si da poi il comando usando il campo COMMAND del terminale in lettura,
            si blocca dunque il processo al corretto semaforo di interrupt per il device*/
        setSTATUS(getSTATUS() & ~IECON);
        termRegRead->recv_command = 2;
        opStatusRead = SYSCALL(IOWAIT, TERMINT, sPtr->sup_asid - 1, 1);
        setSTATUS(getSTATUS() | IECON);

        /*  si estrae il carattere letto dal campo STATUS in lettura e
            lo si scrive nell'indirizzo di memoria richiesto*/
        receivedChar = (opStatusRead & 0xFF00) >> 8;
        *stringAddr = receivedChar;

        /*  se l'operazione non è andata a buon fine, si esce dal ciclo*/
        if((opStatusRead & 0xFF) != 5){
            break;
        }

        /*  si incrementa il numero di caratteri letti e si procede
            all'indirizzo successivo*/
        count++;
        stringAddr++;
        
    }

    /*  se l'intera trasmissione ha avuto successo, si scrive nel registro di ritorno
        il numero di caratteri letti, altrimenti il negato dello stato di errore*/
    if ( (opStatusRead & 0xFF) == 5) {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = count;

    } else {
        sPtr->sup_exceptState[GENERALEXCEPT].reg_v0 = ~opStatusRead;
    }

    /*  si libera l'accesso al terminale in lettura*/
    SYSCALL(VERHOGEN, (int)&(supDevSem[3][sPtr->sup_asid - 1]), 0, 0);

    /*  si riprende l'esecuzione del U-proc*/
    LDST(&(sPtr->sup_exceptState[GENERALEXCEPT]));

}

/*  funzione che esegue il compito della program trap*/
void trapHandler(int supSem, support_t *sPtr){

    /*  si verifica se lo U-proc detiene il controllo su qualche semaforo di supporto
        per i device, e in tal caso si effettua una SYS4*/
    for(int i = 0; i < 4; i++){
        if(!(supDevSem[i][supSem])){
            SYSCALL(VERHOGEN, (int)&(supDevSem[i][supSem]), 0, 0);
        }   
    }

    /*  se la program trap è stata richiesta con uno specifico numero di SYSCALL (20)
        signfica che lo U-proc da eliminare detiene in controllo del semaforo della swap
        pool table, e in tal caso esso viene liberato*/
    if(sPtr->sup_exceptState[GENERALEXCEPT].reg_a0 == BLOCKEDTRAP){
        SYSCALL(VERHOGEN, (int)&(swapPoolSem), 0, 0);
    }

    /*  si chiama la funzione che definisce SYSCALL 9*/
    terminate(sPtr);
}


