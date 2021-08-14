#include "pandos_const.h"
#include "pandos_types.h"
#include "sysSupport.h"
#include "phase2/interrupt.h"
#include "initProc.h"
#include <umps3/umps/libumps.h>

/*  dichiarazioni globali della variabili di supporto
    tra cui la Swap Pool Table*/
swap_t swapPoolTable[POOLSIZE];

/*  variabile che punta all'indirizzo del primo frame della swap pool*/
static memaddr swapPoolPtr;

/*  semaforo che regola l'accesso alla swap pool*/
int swapPoolSem;

/*  variabile usata dall'algoritmo di paging per determinare
    il frame*/
static int frameNo;

swap_t *pagingAlgo(memaddr *frame, int blockNo);

/*  i semafori di supporto sono così strutturati: (SOGGETTO AD EVENTUALE COMBIAMENTO)
    i semafori supDevSem[0][0...7] definiscono i device flash (in ordine)
    i semafori supDevSem[1][0...7] definiscono i printer (in ordine) 
    i semafori supDevSem[2][0...7] definiscono i terminali in scrittura (in ordine)
    i semafori supDevSem[3][0...7] definiscono i terminali in lettura (in ordine)*/
int supDevSem[4][8];

/*  funzione che calcola l'ultimo frame di memoria occupato dal sistema operativo
    così da inizializzare la Swap Pool dal primo frame di RAM disponibile*/
memaddr swapPoolBegin(){
    return *((memaddr *)(KERNELSTACK + 0x0018)) + *((memaddr *)(KERNELSTACK + 0x0024));
}

/*  funzione che inizializza ad 1 i semafori di supporto dei dispositivi
    Flash, Printer e Terminal*/
void initSupSem(){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 8; j++){
            supDevSem[i][j] = 1;
        }
    }
}

/*  funzione chiamata dal instatiatorProcess, inizializza tutte le variabili di supporto*/
void initSwapPool(){
    frameNo = -1;
    swapPoolSem = 1;
    swapPoolPtr = swapPoolBegin();

    /*  inizializzazione dei valori della swap pool table come vuoti, cioè con ASID negativo*/
    for(int i = 0; i < POOLSIZE; i++){
        swapPoolTable[i].sw_asid = -1;
        swapPoolTable[i].sw_pageNo = 0;
        swapPoolTable[i].sw_pte = NULL;
    }
}

/*  funzione che contiene l'algoritmo di paging, viene calcolato l'indirizzo
    del frame della swap pool da utilizzare come e restituito l'indice
    di tale frame nella Swap Pool Table*/
swap_t *pagingAlgo(memaddr *frame, int blockNo){

    /*  il numero del frame non può eccedere POOLSIZE/ / 2 */
    if(frameNo == ((POOLSIZE / 2) + 1)){
        frameNo = -1;
    }

    /* il numero del frame viene incrementato linearmente*/
    frameNo++;

    /*  a seconda del tipo di frame del U-proc da caricare in memoria,
        viene scelto una porzione della swap pool diversa. La metà inferiore
        è dedicata ai U-proc frame di codice mentre la metà superiore mantiene
        i U-proc frame di stack*/
    if(blockNo != 31){
        *frame = swapPoolPtr + (0x1000 * (frameNo % (POOLSIZE / 2)));
        return &(swapPoolTable[frameNo % (POOLSIZE / 2)]);
    }
    else{
        *frame = swapPoolPtr + (0x1000 * ((frameNo + (POOLSIZE / 2)) % POOLSIZE));
        return &(swapPoolTable[(frameNo + (POOLSIZE / 2)) % POOLSIZE]);
    }
}

/*  funzione che aggiorna il TLB*/
void updateTLB(pteEntry_t *entry){

    /*  si effettua una ricerca della Entry nel TLB*/
    setENTRYHI(entry->pte_entryHI);
    TLBP();

    /*  TLBP potrebbe manipolare EntryHi, quindi viene
        caricato nuovamente*/
    setENTRYHI(entry->pte_entryHI);
    setENTRYLO(entry->pte_entryLO);

    /*  se la entry viene trovata, la si aggiorna
        altrimenti si sceglie una entry random*/
    if(!(getINDEX() & PRESENTFLAG)){
        TLBWI();
    }
    else{
        TLBWR();
    }
}

/*  funzione che legge dal Flash device e carica nella RAM, oppure scrive nel
    Flash device leggendo dalla RAM, il parametro "flag" indica quale delle
    due operazioni eseguire*/
void RWflash(int blockNo, memaddr ramAddr, int flashDevNo, int flag){

    /*  si blocca l'accesso dal device Flash*/
    SYSCALL(PASSEREN, (int)&(supDevSem[0][flashDevNo]), 0, 0);

    /*  viene estratto il registro del Flash device associato al processo*/
    dtpreg_t *devReg;
    devReg = (dtpreg_t *) getDevReg(4, flashDevNo);

    /*  viene indicato al flash device da quel di memoria fisica bisogna
        leggere o scrivere*/
    devReg->data0 = ramAddr;

    /*  si disabilitano gli interrupt per eseguire l'operazione in modo atomico*/
    setSTATUS(getSTATUS() & ~IECON);

    /*  viene dato il comando al flash device*/
    devReg->command = (blockNo << 8) | flag;

    /*  si blocca il U-proc sul semaforo di interrupt del Flash device associato*/
    SYSCALL(IOWAIT, FLASHINT, flashDevNo, 0);
    
    /*  una volta sbloccato, si abilitano gli interrupt*/
    setSTATUS(getSTATUS() | IECON);

    /*  se l'operazione non è andata a buon fine, si attiva una program trap
        richiedendo una syscall inesistente ma con un numero particolare che
        indica al trap Handler che il semaforo della Swap Pool Table deve
        essere liberato*/
    if(devReg->status != 1){
        SYSCALL(BLOCKEDTRAP, 0, 0, 0);
    }

    /*  viene liberato il semaforo del Flash Device*/
    SYSCALL(VERHOGEN, (int)&(supDevSem[0][flashDevNo]), 0, 0);
}

/*  se avviene un page fault, l'viene chiamata la funzione passUpOrDie e si
    esegue questa funzione*/
void Pager(){

    /*  si ottiene la struttura di supporto e si determina la causa dell'eccezione*/
    support_t *sPtr = (support_t *) SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    state_t *exState = &(sPtr->sup_exceptState[PGFAULTEXCEPT]);
    unsigned int exCause;
    exCause = (exState->cause & GETEXECCODE) >> 2;

    /*  pandOS imposta tutte le entry delle page table come read-writable,
        se un eccezzione TLB Modification viene alzato, si termina il processo
        con un program trap*/
    if(exCause == TLBMODEXC ){
        SYSCALL(REGULARTRAP, 0, 0, 0);
    }

    /*  viene richiesto l'accesso alla swap pool table*/
    SYSCALL(PASSEREN, (int)&(swapPoolSem), 0, 0);

    /*  si estrae l'indice della page table del U-proc da EntryHi
        e si chiama l'algoritmo di paging per individuare il frame
        della swap pool da usare*/
    int excPageNo;
    memaddr frameAddr;
    swap_t *poolTablePtr;

    excPageNo = (exState->entry_hi & GETPAGENO) >> VPNSHIFT;

    if(excPageNo == 0x3FFFF){
        excPageNo = 31;
    }

    poolTablePtr = pagingAlgo(&(frameAddr), excPageNo);
    
    /*  se il frame non è libero, bisogna aggiornale l'eventuale entry del TLB
        e ricopiare il contenuto del frame del flash device associato al U-proc che
        detiene il frame appena estratto*/
    if(poolTablePtr->sw_asid != -1){

        /*  si disabilitano gli interrupt per eseguire atomicamente
            l'aggioranamento del TLB*/
        setSTATUS(getSTATUS() & ~IECON);

        /*  si invalida la entry nella page table*/
        poolTablePtr->sw_pte->pte_entryLO &= ~VALIDON;

        /*  si cerca la entry nel TLB*/
        setENTRYHI(poolTablePtr->sw_pte->pte_entryHI);
        TLBP();

        /*  se presente, la si aggiorna invalidandola*/
        if(!(getINDEX() & PRESENTFLAG)){
            setENTRYHI(poolTablePtr->sw_pte->pte_entryHI);
            setENTRYLO(poolTablePtr->sw_pte->pte_entryLO);
            TLBWI();
        }

        /*  si riabilitano gli interrup*/
        setSTATUS(getSTATUS() | IECON);
        
        /*  si estrae l'indice del blocco nel Flash device assiociato al frame della swap pool*/
        int blockToUpload, PFNtoUpload;

        blockToUpload = (poolTablePtr->sw_pte->pte_entryHI & GETPAGENO) >> VPNSHIFT;
        if(blockToUpload == 0x3FFFF){
            blockToUpload = 31;
        }

        /*  si estrae l'indirizzo fisico del frame nella swap pool*/
        PFNtoUpload = poolTablePtr->sw_pte->pte_entryLO & 0xFFFFF000;

        /*  si riscrive il contenuto del frame nel flash device*/
        RWflash(blockToUpload, PFNtoUpload, poolTablePtr->sw_asid - 1, FLASHWRITE);

    }
    
    /*  si copia il contenuto del flash device nel frame della swap pool*/
    RWflash(excPageNo, frameAddr, sPtr->sup_asid - 1, FLASHREAD);

    /*  si aggiorna la entry della swap pool table*/
    poolTablePtr->sw_asid = sPtr->sup_asid;
    poolTablePtr->sw_pageNo = excPageNo;
    poolTablePtr->sw_pte = &(sPtr->sup_privatePgTbl[excPageNo]);

    /*  si disabilitano gli intertuper per aggioranere atomicamente la page table
        e il TLB*/
    setSTATUS(getSTATUS() & ~IECON);
    
    sPtr->sup_privatePgTbl[excPageNo].pte_entryLO = frameAddr | VALIDON | DIRTYON;
    updateTLB(&(sPtr->sup_privatePgTbl[excPageNo]));

    setSTATUS(getSTATUS() | IECON);

    /*  viene liberato l'accesso allo swap pool table*/
    SYSCALL(VERHOGEN, (int)&(swapPoolSem), 0, 0);
    
    /*  si riprende l'esecuzione del processo corrente*/
    LDST(exState);

}