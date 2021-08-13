#include "exceptions.h"
#include "scheduler.h"
#include "init.h"
#include "syscall.h"
#include <pandos_types.h>
#include <pandos_const.h>

/*puntatore a BIOSDATAPAGE
 *usato in molte altre funzioni
*/
state_t *currentState = BIOSDATAPAGE;
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

/* TLB-Refill Handler */
/* One can place debug calls here, but not calls to print */
void uTLB_RefillHandler () {

	unsigned int entropy;
	entropy = currentState->entry_hi;
	pteEntry_t *eccolo;

	for(int i = 0; i < USERPGTBLSIZE; i++){
		if(entropy == currentProcess->p_supportStruct->sup_privatePgTbl[i].pte_entryHI){
			eccolo = &(currentProcess->p_supportStruct->sup_privatePgTbl[i]);
			break;
		}
	}
	
	static int entrHI, entrLO;
	entrHI = setENTRYHI(eccolo->pte_entryHI);
	entrLO = setENTRYLO(eccolo->pte_entryLO);
	TLBWR();

	LDST ((state_t *) 0x0FFFF000);
	
}


/*funzione per copiare strutture*/
void memcpy(memaddr *src, memaddr *dest, unsigned int bytes){
    
    for(int i = 0; i < (bytes/4); i++){
        *dest = *src;
        dest++;
        src++;
    }
}

