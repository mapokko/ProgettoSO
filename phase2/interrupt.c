#include "exceptions.h"
#include "initial.h"
#include "scheduler.h"
#include "pcb.h"
#include "asl.h"
#include <pandos_types.h>
#include <pandos_const.h>

memaddr *getDevReg(int devNumber, int interruptingDeviceNumber);

/*viene gestito l'effettivo linea di interrupt, che non potrà mia essere 0 perché non compoete al progetto*/
void interruptHandler(){
	static unsigned int interruptCode;
	interruptCode = currentState->cause & CLEAREXECCODE;

	if(interruptCode & LOCALTIMERINT){
		PLTHandler();
		return;
	}
	else if(interruptCode & TIMERINTERRUPT){
		timerHandler();
		return;
	}
	else if(interruptCode & DISKINTERRUPT){
        IODeviceHandler(DISKINT);
	}
	else if(interruptCode & FLASHINTERRUPT){
        IODeviceHandler(FLASHINT);
	}
	else if(interruptCode & (FLASHINTERRUPT << 1)){ /*dovrebbe essere l'interrupt del Network Device*/
        IODeviceHandler(NETWINT);
	}
	else if(interruptCode & PRINTINTERRUPT){
        IODeviceHandler(PRNTINT);
	}
	else if(interruptCode & TERMINTERRUPT){
		terminalHandler();
		return;
	}
}

/*gestore della linea di interrupt 1, che identifica cun interrupt dal processor local timer*/
void PLTHandler(){
	updateCPUtime();
	memcpy(currentState, &(currentProcess->p_s), sizeof(state_t));
	
	insertProcQ(&readyQ, currentProcess);
	setTIMER(100000);
	scheduler();
}

/*linea di interrupt 2, global timer*/
void timerHandler(){
	
	while(pseudoClock < 0){
		verhogen(&pseudoClock);
	}
	
	LDIT(PSECOND);
	checkCurrent();
	LDST(currentState);
	
}

void IODeviceHandler(int devNumber){
    static unsigned int interruptingDeviceNum, status;
    static dtpreg_t *deviceRegister;
    static pcb_t *unblocked;
     
    interruptingDeviceNum = getLine(getBitmap(devNumber));
	deviceRegister = getDevReg(devNumber, interruptingDeviceNum);

    status = deviceRegister->status;
    
    unblocked = verhogen(&(devSem[devNumber - 3][interruptingDeviceNum]));
    if(unblocked != NULL){
        unblocked->p_s.reg_v0 = status;
    }
  
    deviceRegister->command = 1;

    checkCurrent();
    LDST(currentState);
}

/*linea di interrupt 7, gestore terminale*/
void terminalHandler(){
	static unsigned int interruptingDeviceNum, status;
    static termreg_t *terminalRegister;
	static pcb_t *unblockedPcb;

	interruptingDeviceNum = getLine(getBitmap(TERMINT));
	terminalRegister = getDevReg(TERMINT, interruptingDeviceNum);

	status = terminalRegister->recv_status;

    for(int i = 0; i <= 1; i++){
        if(i == 0){
            terminalRegister->recv_command = 1;
        }
        else{
            terminalRegister->transm_command = 1;
        }
		unblockedPcb = verhogen(&(terSem[i][interruptingDeviceNum]));
		if(unblockedPcb != NULL){
		    unblockedPcb->p_s.reg_v0 = status;
        }
	
        status = terminalRegister->transm_status;
    }
	
	checkCurrent();
	LDST(currentState);

}



void checkCurrent(){
	if(currentProcess == NULL){
		scheduler();
	}
}

/*restiusce il bitmap del numero del dispositivo passato, flag serve per dire quale bitmap(interrupt o installed)*/
int getBitmap(int devNumber){
	return bus_devReg_Area->interrupt_dev[devNumber - 3];
}

/*prende un bitmap, restituisce la prima istanza del primo dispositivo su cui pende interrupt*/
int getLine(int bitmap){
	static unsigned int scanner, line;
	scanner = 1;
	line = 0;

	while(!(bitmap & scanner)){
		scanner *= 2;
		line++;
	}

	return line;
}

/*preso un dispositivo e  degli interrupt associato, restituisce l'indirizzo del primo bitmap*/
memaddr *getDevReg(int devNumber, int interruptingDeviceNumber){
	return DEVICEREGISTERBASE + ((devNumber - 3) * 0x80) + ((interruptingDeviceNumber) * 0x10);
}