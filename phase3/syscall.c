#include "exceptions.h"
#include "init.h"
#include "scheduler.h"
#include <pandos_types.h>
#include <pandos_const.h>

pcb_t *verhogen(int *semAddrV);

/*gestore delle SYSCALL*/
void syscallHandler(){
    /*incremento del program counter
     *per evitare loop infinito
    */
    currentState->pc_epc += 4;
	
    /*cedo controllo alla SYSCALL richiesta*/
	switch (currentState->reg_a0){
		case CREATEPROCESS: 
		create_Process(currentState->reg_a1, currentState->reg_a2);
		break;
		case TERMPROCESS: 
		terminate_Process();
		break;
		case PASSEREN: 
		passeren(currentState->reg_a1);
		break;
		case VERHOGEN: 
		verhogenExternal(currentState->reg_a1);
		break;
        case IOWAIT: 
		wait_for_IO_Device(currentState->reg_a1, currentState->reg_a2, currentState->reg_a3);
		break;
        case GETTIME: 
		get_CPU_time();
		break;
        case CLOCKWAIT: 
		wait_clock();
		break;
        case GETSUPPORTPTR: 
		get_support_data();
		break;
		default: 
		passUp_orDie(GENERALEXCEPT);
		break;
	}
}


/*SYSCALL 1*/
void create_Process(state_t *statep, support_t *supportp){
    /*alloco nuovo pcb*/
	pcb_t *newPcb;
	newPcb = allocPcb();

    /*se non è possibile allocare
     *comunica al processo e
     *restituisci controllo
    */
	if(newPcb == NULL){
        currentState->reg_v0 = -1;
        LDST(currentState);
	}

    /*copio nel nuovo pcb i dati passati per parametro*/
	memcpy(statep, &(newPcb->p_s), sizeof(state_t));
	newPcb->p_supportStruct = supportp;

    /*inizializzo campi del pcb*/
	newPcb->p_time = 0;
	newPcb->p_semAdd = NULL;

    /*rendo il nuovo pcb figlio del pcb corrente
     *e inserisco il pcb nella readyQ
    */
	insertChild(currentProcess, newPcb);
	insertProcQ(&readyQ, newPcb);

    /*comunico al processo corrente
     *che allocazione è OK
     *e incremento processi attivi
    */
	currentState->reg_v0 = 0;
	processCount++;

    /*restituisco controllo all'esecuzione corrente*/
	LDST(currentState);
}

/*SYSCALL 2*/
void terminate_Process(){
	
    /*chiamata della funzione ricorsiva*/
	terminate_ProcessRec(currentProcess->p_child);

    /*rimuovo il pcb corrente
    *dalla lista dei figli di suo padre e
    *lo restituisco alla lista dei pcb liberi
    */
    outChild(currentProcess);
	freePcb(currentProcess);

    /*decremento numeri di processi
     *attivi e cedo controllo
     *allo scheduler
    */
	processCount--;
	scheduler();
}
/*ricorsione della SYSCALL 2*/
void terminate_ProcessRec(pcb_t *pcbPointer){
    /*condizione di terminazione della ricorsione*/
	if(pcbPointer == NULL){
		return;
	}

    /*chiamata ricorsiva sul figlio*/
	terminate_ProcessRec(pcbPointer->p_child);
    /*chiamata ricorsiva sul fratello*/
	terminate_ProcessRec(pcbPointer->p_next_sib);

    /*rimuovo il pcb dalla sua lista di 
     *fratello e dalla readyQ
    */
	outChild(pcbPointer);
	outProcQ(&readyQ, pcbPointer);

    /*rimuovo il pcb dalla sua queue
     *dei pcb bloccati
     *se la rimozione avviene aggiorno il
     *semaforo e il numero di processi bloccati
    */
	if(outBlocked(pcbPointer) != NULL){
		*(pcbPointer->p_semAdd) = *(pcbPointer->p_semAdd) + 1;
		blockedCount--;
	}

    /*restituisco il pcb
     *alla lista libera e
     *aggiorno il numero di
     processu attivi
    */
	freePcb(pcbPointer);
	processCount--;
}

/*SYSCALL 3*/
void passeren(int* semAddrP){
    /*decremento semaforo*/
	*semAddrP -= 1;
    /*se la risorsa non è disponibile
     *blocca il pcb nella queue di semd_t
    */
	if(*semAddrP < 0){
        /*copio stato corrente per riprendere l'esecuzione*/
        memcpy(currentState, &currentProcess->p_s, sizeof(state_t));
        /*aggiorno il campo p_time*/
		updateCPUtime();

        /*blocco il pcb, se non è possibile, PANIC*/
		if(insertBlocked(semAddrP, currentProcess)){
            PANIC();
        }
        /*incremento numero di processi bloccati
         *e chiamo scheduler
        */
		blockedCount++;
		scheduler();
	}
    /*se la ricorda è disponibile, continua
     *l'esecuzione nella CS
    */
	LDST(currentState);
}

/*SYSCALL 4*/
void verhogenExternal(int *semAddrV){
    /*chiamata di verhogene e
     *restituzione del controllo
    */
	verhogen(semAddrV);
	LDST(currentState);
}

/*V operation, usata anche internamente*/
pcb_t *verhogen(int *semAddrV){
    /*incremento semaforo*/
	*semAddrV += 1;

    /*se ci sono processi bloccati
     *sblocco e lo metto nella readyQ
    */
	if(*semAddrV <= 0){
        /*rimuovo dai bloccati*/
		pcb_t *unblocked;
		unblocked = removeBlocked(semAddrV);
        /*inserisco nella readyQ e decremento i bloccati*/
		insertProcQ(&readyQ, unblocked);
		blockedCount--;
		return unblocked;
		
	}
	return NULL;
}


/*SYSCALL 5*/
void wait_for_IO_Device(int devNumber, int devLine, int termReadFlag){
	
    /*individuo il semaforo richiesto e chiamo passeren()*/
	if(devNumber != 7){
		passeren(&(devSem[devNumber - 3][devLine]));
	}
    /*controllo speciale per capire quale semaforo del terminale*/
	else if(termReadFlag){
		passeren(&(terSem[0][devLine]));
	}
	else{
		passeren(&(terSem[1][devLine]));
	}
}

/*SYSCALL 6*/
void get_CPU_time(){
    /*comunico al processo corrente il p_time 
	 *a cui sommo il tempo passato per la gestione
	 *dell'eccezione corrente
     *e restituisco il controllo
    */
   	int currTOD;
	STCK(currTOD);
	currentState->reg_v0 = currentProcess->p_time + (currTOD - excTOD);
	LDST(currentState);
}

/*SYSCALL 7*/
void wait_clock(){
    /*semplice passeren sul semaforo
    *del interval timer
    */
	passeren(&pseudoClock);
}

/*SYSCALL 8*/
void get_support_data(){
    /*passo l'indirizzo della struttura di supporto
     *e restituisco il controllo
    */
	currentState->reg_v0 = currentProcess->p_supportStruct;
	LDST(currentState);
}

/*pass up or die*/
void passUp_orDie(int contextPosition){
    /*se non ha struttura di supporto, termina processo*/
	if(currentProcess->p_supportStruct == NULL){
		terminate_Process();
	}
    /*copio lo stato corrente nella posizione corretta*/
	memcpy(currentState, &(currentProcess->p_supportStruct->sup_exceptState[contextPosition]), sizeof(state_t));
    
    /*estraggo SP, status e PC dalla struttura di controllo*/
    unsigned int stackPtr, status, progCounter;
    stackPtr = currentProcess->p_supportStruct->sup_exceptContext[contextPosition].c_stackPtr;
    status = currentProcess->p_supportStruct->sup_exceptContext[contextPosition].c_status;
    progCounter = currentProcess->p_supportStruct->sup_exceptContext[contextPosition].c_pc;

    /*cedo il controllo al livello di supporto*/
	LDCXT(stackPtr, status, progCounter);

}

/*funzione ausiliaria per aggiornare il tempo
 *occupato dal pcb
*/
void updateCPUtime(){
	unsigned int tod;
	STCK(tod);
	currentProcess->p_time += tod - processStartTime;
}