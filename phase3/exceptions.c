#include "exceptions.h"
#include "scheduler.h"
#include <pandos_types.h>
#include <pandos_const.h>

/*puntatore a BIOSDATAPAGE
 *usato in molte altre funzioni
*/
state_t *currentState = BIOSDATAPAGE;

/*gestore delle eccezioni*/
void kernelExcHandler(){

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

/*funzione per copiare strutture*/
void memcpy(memaddr *src, memaddr *dest, unsigned int bytes){
    
    for(int i = 0; i < (bytes/4); i++){
        *dest = *src;
        dest++;
        src++;
    }
}