#include "exceptions.h"
#include "initial.h"
#include "scheduler.h"
#include "pcb.h"
#include "asl.h"
#include <pandos_types.h>
#include <pandos_const.h>
//#include ‘/usr/include/umps3/umps/libumps.h’

state_t *currentState = BIOSDATAPAGE;


void kernelExcHandler(){
	
    static unsigned int excCode;
    excCode = (currentState->cause & GETEXECCODE) >> 2;
    switch (excCode){
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
		
		break;
	}
    

}

