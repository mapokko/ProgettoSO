#include "exceptions.h"
#include "initial.h"
#include "scheduler.h"
#include "pcb.h"
#include "asl.h"
#include <pandos_types.h>
#include <pandos_const.h>
//#include ‘/usr/include/umps3/umps/libumps.h’

static state_t *currentState = BIOSDATAPAGE;
static unsigned int interruptStartTime;

// void initDevSem(){
// 	static int installedLine, scanner = 1;

// 	for(int i = 3; i < 7; i++){
// 		installedLine = getBitmap(i, INSTALLEDBITMAP);
		
// 		for(int j = 0; j < 8; j++){
// 			if(installedLine & scanner){
// 				allocASL(&(devSem[i - 3][j]));
// 			}
// 			scanner *= 2;
// 		}
// 		scanner = 1;
// 	}

// 	installedLine = getBitmap(7, INSTALLEDBITMAP);

// 	for(int j = 0; j < 8; j++){
// 		if(installedLine & scanner){
// 			//semaforo per receive del terminale
// 			allocASL(&(terSem[0][j]));
// 			//semaforo per trasmit del terminale
// 			allocASL(&(terSem[1][j]));
// 		}
// 		scanner *= 2;
// 	}

// 	allocASL(&pseudoClock);
// }

void uTLB_RefillHandler () {

	setENTRYHI(0x80000000);
	setENTRYLO(0x00000000);
	TLBWR();
	LDST ((state_t *) 0x0FFFF000);
}

void kernelExcHandler(){
	interruptStartTime = getTIMER();
	
    static unsigned int excCode;
    excCode = (currentState->cause & GETEXECCODE) >> 2;
    switch (excCode){
		case 0: 
		interruptHandler();
		break;
		case 1 ... 3: 
		
		break;
		case 4 ... 7: 
		
		break;
        case 8: 
        syscallHandler();
		break;
        case 9 ... 12: 
    
		break;
		default: 
		
		break;
	}
    

}

/*comincia la gestione degli INTERRUPT----------------------------------------------------------------------------------------------*/

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
	
	memcpy(currentState, &(currentProcess->p_s), sizeof(state_t));
	
	
	insertProcQ(&readyQ, currentProcess);
	setTIMER(100000);
	scheduler();
}

/*linea di interrupt 2, global timer*/
void timerHandler(){
	static semd_t *timerSem;
	timerSem = isInserted(&pseudoClock);
	static pcb_t *pcbPtr;

	if(timerSem != NULL){
		while(timerSem->s_procQ != NULL){
			pcbPtr = removeBlocked(timerSem);
			insertProcQ(&readyQ, pcbPtr);
		}
	}
	

	pseudoClock = 0;
	LDIT(PSECOND);
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
			insertProcQ(&readyQ, unblockedPcb);
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
			insertProcQ(&readyQ, unblockedPcb);
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

/*preventivamente*/
void setPcbTimer(){
	currentProcess->p_time += 5000 - interruptStartTime;
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


/*comincia la gestione dei SYSCALL--------------------------------------------------------------------------------------------------*/

void syscallHandler(){
	static unsigned int syscallCode;
	syscallCode = currentState->reg_a0;

	switch (syscallCode){
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
		verhogenWrapper(currentState->reg_a1);
		break;
        case IOWAIT: 
		wait_for_IO_Device(currentState->reg_a1, currentState->reg_a2, currentState->reg_a3);
		break;
        case GETTIME: 
		get_CPU_time();
		break;
        case CLOCKWAIT: 
		
		break;
        case GETSUPPORTPTR: 
    
		break;
		default: 
		
		break;
	}
}

void updateCPUtime(){
	currentProcess->p_time += (5000 - interruptStartTime);
}

void create_Process(state_t *statep, support_t *supportp){
	static pcb_t *newPcb;
	newPcb = allocPcb();
	currentState->pc_epc = currentState->pc_epc + 4;

	if(newPcb == NULL){
		currentState->reg_v0 = -1;
		LDST(currentState);
	}

	memcpy(statep, &(newPcb->p_s), sizeof(state_t));

	// if(supportp != -1 ){
	// 	newPcb->p_supportStruct = supportp;
	// }
	// else{
	// 	newPcb->p_supportStruct = NULL;
	// }
	
	newPcb->p_supportStruct = supportp;

	newPcb->p_time = 0;
	newPcb->p_semAdd = NULL;
	newPcb->p_s.status = IEPON | TEBITON | IMON; //non so se questo status sia corretto per i nuovi pcb
	insertChild(currentProcess, newPcb);
	insertProcQ(&readyQ, newPcb);
	currentState->reg_v0 = 0;
	LDST(currentState);
}

void terminate_Process(){
	terminate_ProcessRec(currentProcess->p_child);
	freePcb(currentProcess);
	scheduler();
}

void terminate_ProcessRec(pcb_t *child){
	if(child == NULL){
		return;
	}
	terminate_ProcessRec(child->p_next_sib);
	outBlocked(child);
	outProcQ(&readyQ, child);
	freePcb(child);
}

void passeren(int* semAddrP){
	*semAddrP -= 1;

	currentState->pc_epc += 4;
	memcpy(currentState, &currentProcess->p_s, sizeof(state_t));
	if(*semAddrP < 0){
		updateCPUtime();
		insertBlocked(semAddrP, currentProcess);
		blockedCount++;
		scheduler();
	}

	LDST(currentState);
}

pcb_t *verhogen(int *semAddrV){
	*semAddrV += 1;
	if(*semAddrV <= 0){
		static pcb_t *unblocked;
		unblocked = removeBlocked(semAddrV);
		blockedCount--;
		return unblocked;
	}
	return NULL;
}

void verhogenWrapper(int *semAddrV){
	static pcb_t *vPcb;
	vPcb = verhogen(semAddrV);
	currentState->pc_epc += 4;
	if(vPcb != NULL){
		insertProcQ(&readyQ, vPcb);
	}
	
	LDST(currentState);
}

void wait_for_IO_Device(int devNumber, int devLine, int termReadFlag){
	

	if(devNumber != 7){
		passeren(&(devSem[devNumber - 3][devLine]));
	}
	else if(termReadFlag){
		passeren(&(terSem[0][devLine]));
	}
	else{
		passeren(&(terSem[1][devLine]));
	}
}

void get_CPU_time(){
	updateCPUtime();
	currentState->reg_v0 = currentProcess->p_time;
	currentState->pc_epc += 4;
	LDST(currentState);
}