#include "exceptions.h"
#include "initial.h"
#include "scheduler.h"
#include "pcb.h"
#include "asl.h"
#include <pandos_types.h>
#include <pandos_const.h>

void syscallHandler(){
	static unsigned int syscallCode;
	syscallCode = currentState->reg_a0;
    currentState->pc_epc += 4;
	
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

void updateCPUtime(){
	unsigned int tod;
	STCK(tod);
	currentProcess->p_time += tod - processStartTime;
}

void create_Process(state_t *statep, support_t *supportp){
	static pcb_t *newPcb;
	newPcb = allocPcb();

	if(newPcb == NULL){
        currentState->reg_v0 = -1;
        LDST(currentState);
	}

	memcpy(statep, &(newPcb->p_s), sizeof(state_t));
	newPcb->p_supportStruct = supportp;

	newPcb->p_time = 0;
	newPcb->p_semAdd = NULL;
	newPcb->p_s.status = IEPON | TEBITON | IMON; //non so se questo status sia corretto per i nuovi pcb

	insertChild(currentProcess, newPcb);
	insertProcQ(&readyQ, newPcb);

	currentState->reg_v0 = 0;
	processCount++;

	LDST(currentState);
}

void terminate_Process(){
	
	terminate_ProcessRec(currentProcess->p_child);

    outChild(currentProcess);
	freePcb(currentProcess);

	processCount--;
	scheduler();
}

void terminate_ProcessRec(pcb_t *child){
	if(child == NULL){
		return;
	}

	terminate_ProcessRec(child->p_child);
	terminate_ProcessRec(child->p_next_sib);

	outChild(child);
	outProcQ(&readyQ, child);

	if(outBlocked(child) != NULL){
		*(child->p_semAdd) = *(child->p_semAdd) + 1;
		blockedCount--;
	}

	freePcb(child);
	processCount--;
}

void passeren(int* semAddrP){
	*semAddrP -= 1;

	if(*semAddrP < 0){
        memcpy(currentState, &currentProcess->p_s, sizeof(state_t));
		updateCPUtime();
		if(insertBlocked(semAddrP, currentProcess)){
            PANIC();
        }
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
		insertProcQ(&readyQ, unblocked);
		blockedCount--;
		return unblocked;
		
	}
	return NULL;
}

void verhogenExternal(int *semAddrV){
	verhogen(semAddrV);
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
	currentState->reg_v0 = currentProcess->p_time;
	LDST(currentState);
}

void wait_clock(){
	passeren(&pseudoClock);
}

void get_support_data(){
	currentState->reg_v0 = currentProcess->p_supportStruct;
	LDST(currentState);
}

void passUp_orDie(int contextNumber){
	if(currentProcess->p_supportStruct == NULL){
		terminate_Process();
	}

	memcpy(currentState, &(currentProcess->p_supportStruct->sup_exceptState[contextNumber]), sizeof(state_t));
    unsigned int stackPtr, status, progCounter;

    stackPtr = currentProcess->p_supportStruct->sup_exceptContext[contextNumber].c_stackPtr;
    status = currentProcess->p_supportStruct->sup_exceptContext[contextNumber].c_status;
    progCounter = currentProcess->p_supportStruct->sup_exceptContext[contextNumber].c_pc;

	LDCXT(stackPtr, status, progCounter);

}