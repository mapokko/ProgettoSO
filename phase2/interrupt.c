#include "exceptions.h"
#include "initial.h"
#include "scheduler.h"
#include "pcb.h"
#include "asl.h"
#include <pandos_types.h>
#include <pandos_const.h>

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

	}
	else if(interruptCode & FLASHINTERRUPT){

	}
	else if(interruptCode & (FLASHINTERRUPT << 1)){ /*dovrebbe essere l'interrupt del Network Device*/

	}
	else if(interruptCode & PRINTINTERRUPT){

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

/*linea di interrupt 7, gestore terminale*/
void terminalHandler(){
	static memaddr *terminalRegister;
	static unsigned int bitMapLine, status;
	static semd_t *semaphore;
	static pcb_t *unblockedPcb;

	bitMapLine = getLine(getBitmap(TERMINT, INTERRUPTBITMAP));
	terminalRegister = getDevReg(TERMINT, bitMapLine);

	status = *(terminalRegister + 2);
	status = (status & 0xff);

	if(status == 5){
		*(terminalRegister + 3) = 1;
		unblockedPcb = verhogen(&(terSem[1][bitMapLine - 1]));
		if(unblockedPcb != NULL){
			unblockedPcb->p_s.reg_v0 = status;
			//unblockedPcb->p_s.pc_epc = unblockedPcb->p_s.reg_t9 = unblockedPcb->p_s.pc_epc + 4;
			//insertProcQ(&readyQ, unblockedPcb);
		}

	}

	status = *(terminalRegister);
	status = (status & 0xff);

	if(status == 5){
		*(terminalRegister + 1) = 1;
		unblockedPcb = verhogen(&(terSem[0][bitMapLine - 1]));
		if(unblockedPcb != NULL){
			unblockedPcb->p_s.reg_v0 = status;
			//unblockedPcb->p_s.pc_epc = unblockedPcb->p_s.reg_t9 = unblockedPcb->p_s.pc_epc + 4;
			//insertProcQ(&readyQ, unblockedPcb);
		}
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
int getBitmap(int devNumber, int flag){

	static unsigned int *interrupting;

	if(flag){
		interrupting = (memaddr) INTERRUPTINGBASEADDRESS;
	}
	else{
		interrupting = (memaddr) INSTALLEDBASEADDRESS;
	}
	
	interrupting += devNumber - 3;


	return *interrupting;
}

/*prende un bitmap, restituisce la prima istanza del primo dispositivo su cui pende interrupt*/
int getLine(int bitmap){
	static unsigned int scanner, line;
	scanner = 1;
	line = 1;

	while(!(bitmap & scanner)){
		scanner *= 2;
		line++;
	}

	return line;
}

/*preso un dispositivo e  degli interrupt associato, restituisce l'indirizzo del primo bitmap*/
memaddr *getDevReg(int devNumber, int bitMapLine){
	//return DEVICEREGISTERBASE + ((devNumber - 3) * 0x80) + (bitMapLine * 0x10);
	return DEVICEREGISTERBASE + ((devNumber - 3) * 0x80) + ((bitMapLine - 1) * 0x10);
}