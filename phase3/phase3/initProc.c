#include "pandos_const.h"
#include "pandos_types.h"
#include "vmSupport.h"
#include "sysSupport.h"
#include <umps3/umps/libumps.h>

/*  dichiarazione delle variabili globali per la struttura di supporto
    dei processi, la variabile ramtop che contiene l'indirizzo di RAMTOP
    e il semaforo globale su cui si blocca il processo instanziatore*/
static support_t supportTable[UPROCMAX];
static memaddr ramtop;
int masterSem = 0;

/*  inizializza la Page Table del processo*/
void initUPageTable(int ASID){

    /*  assegnamento id entryHi e entryLo*/
    for(int i = 0; i < USERPGTBLSIZE - 1; i++){
        supportTable[ASID].sup_privatePgTbl[i].pte_entryHI = ((0x80000 | i ) << VPNSHIFT) | ((ASID + 1) << ASIDSHIFT);
        supportTable[ASID].sup_privatePgTbl[i].pte_entryLO = DIRTYON;
    }
    /*  l'ultima entry della Page Table deve essere inizializzata per lo stack del processo
        dunque deve avere VPN diverso*/
    supportTable[ASID].sup_privatePgTbl[31].pte_entryHI = (0xBFFFF << VPNSHIFT) | ((ASID + 1) << ASIDSHIFT);
    supportTable[ASID].sup_privatePgTbl[31].pte_entryLO = DIRTYON;

}

/*  inizializza e crea i processi di test*/
void initUProc(){
    int createStatus;

    /*  ogni iterazione del ciclo crea un processo*/
    for(int i = 0; i < UPROCMAX; i++){

        /*  creazione e inizializzazione dello stato del processo*/
        state_t newState;
        newState.pc_epc = newState.reg_t9 = UPROCSTARTADDR;
        newState.reg_sp = USERSTACKTOP;
        newState.status = TEBITON | IMON | IEPON | USERPON;
        newState.entry_hi = (i + 1) << ASIDSHIFT;
        
        /*  inizializzazione della struttura di supporto*/
        supportTable[i].sup_asid = i + 1;
        supportTable[i].sup_exceptContext[PGFAULTEXCEPT].c_pc = (int) Pager;
        supportTable[i].sup_exceptContext[GENERALEXCEPT].c_pc = (int) generalSupHandler;

        /*  lo stack per Page Fault e General Support Exc viene assegnato direttamente nella RAM disponbibile
            immediatamente sotto lo stack del primo processo*/
        supportTable[i].sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = ramtop - ((i + 1) * PAGESIZE * 2) + PAGESIZE;
        supportTable[i].sup_exceptContext[GENERALEXCEPT].c_stackPtr = ramtop - ((i + 1) * PAGESIZE * 2);

        supportTable[i].sup_exceptContext[PGFAULTEXCEPT].c_status = supportTable[i].sup_exceptContext[GENERALEXCEPT].c_status = IMON | IEPON;

        /*  funzione che inizializza la Page Table del processo*/
        initUPageTable(i);

        /*  SYSCALL per creare il processo*/
        createStatus = SYSCALL(CREATEPROCESS, (memaddr) &(newState), (memaddr) &(supportTable[i]), 0);

        /*  se la creazione non è andata a buon fine si eliminano tutti i processi*/
        if(createStatus){
            SYSCALL(TERMPROCESS, 0, 0, 0);
        }
    }
}

/*  funzione eseguita al primo processo, che viene inizializzato in init.c*/
void instantiatorProcess(){
    /*  assegnamento di ramtop*/
    RAMTOP(ramtop);

    /*  inizializzazione dello swap pool e delle variabili collegate*/
    initSwapPool();

    /*  inizializzazione dei semafori di supporto per i dispositivi*/
    initSupSem();

    /*  inizializzazione dei processi di test*/
    initUProc();
    
    /*  al termine di ogni processo, il processo instanziatore riprende l'esecuzione e
        si blocca nuovamente*/
    for(int i = 0; i < UPROCMAX; i++){
        SYSCALL(PASSEREN, (int)&(masterSem), 0, 0);
    }

    /*  una volta che tutti i processi saranno terminati, verrà effettuato un ultimo SYS4 dalla funzione di terminazione
        e l'esecuzione giungerà qui, terminando il processo instanziatore*/
    SYSCALL(TERMPROCESS, 0, 0, 0);
}


