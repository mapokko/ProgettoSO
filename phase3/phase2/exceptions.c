#include "exceptions.h"
#include "scheduler.h"
#include "init.h"
#include "syscall.h"
#include "interrupt.h"
#include <pandos_types.h>
#include <pandos_const.h>

/*puntatore a BIOSDATAPAGE
 *usato in molte altre funzioni
*/
state_t *currentState = (state_t *)BIOSDATAPAGE;
int excTOD;

/*gestore delle eccezioni*/
void kernelExcHandler(){
	STCK(excTOD);
	

	/*estraggo l' excCode e
	 *cedo controllo al gestore specifico
	 */
    switch ((currentState->cause & GETEXECCODE) >> 2){
		case 0: 
		interruptHandler();
		break;
		case 1 ... 3: 
		passUp_orDie(PGFAULTEXCEPT);
		break;
		case 4 ... 7: 
		passUp_orDie(GENERALEXCEPT);
		break;
        case 8: 
        syscallHandler();
		break;
        case 9 ... 12: 
		passUp_orDie(GENERALEXCEPT);
		break;
		default: 
		passUp_orDie(GENERALEXCEPT);
		break;
	}
    

}

/* 	TLB-Refill Handler */
/*	il controllo giunge qui quando si prova ad accedere ad un indirizzo virtuale che
	non ha un corrispettivo nella TLB*/
void uTLB_RefillHandler () {
	pteEntry_t *pgTblEntry;

	/*	si estrae la entry della page table dello U-proc associato all'indirizzo richesto*/
	int index = (currentState->entry_hi & GETPAGENO) >> VPNSHIFT;
	if(index ==  0x3FFFF){
		index = 31;
	}
	
	pgTblEntry = &(currentProcess->p_supportStruct->sup_privatePgTbl[index]);

	/*	si impostano i registri entryHi e entryLo del CP0 e
		si scrivono in una entry random del TLB usndo TLBWR*/
	setENTRYHI(pgTblEntry->pte_entryHI);
	setENTRYLO(pgTblEntry->pte_entryLO);
	TLBWR();

	/*	si restituisce il controllo al U-proc corrente*/
	LDST (currentState);
	
}


/*funzione per copiare strutture*/
void memcpy(memaddr *src, memaddr *dest, unsigned int bytes){
    
    for(int i = 0; i < (bytes/4); i++){
        *dest = *src;
        dest++;
        src++;
    }
}

