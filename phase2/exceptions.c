#include "exceptions.h"
#include "initial.h"
#include "scheduler.h"
#include "pcb.h"
#include "asl.h"
#include <pandos_types.h>
#include <pandos_const.h>
//#include ‘/usr/include/umps3/umps/libumps.h’

static state_t *currentState = BIOSDATAPAGE;

void uTLB_RefillHandler () {

	setENTRYHI(0x80000000);
	setENTRYLO(0x00000000);
	TLBWR();
	LDST ((state_t *) 0x0FFFF000);
}

void kernelExcHandler(){
    static unsigned int excCode;
    excCode = (currentState->cause | GETEXECCODE) >> 2;
    switch (excCode){
		case 0: 
		
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

void interruptHandler(){
	
}

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
		verhogen(currentState->reg_a1);
		break;
        case IOWAIT: 
		
		break;
        case GETTIME: 
		
		break;
        case CLOCKWAIT: 
		
		break;
        case GETSUPPORTPTR: 
    
		break;
		default: 
		
		break;
	}
}

void create_Process(state_t *statep, support_t *supportp){
	static pcb_t *newPcb;
	newPcb = allocPcb();
	//currentState->pc_epc = currentState->pc_epc + 4;

	if(newPcb == NULL){
		currentState->reg_v0 = -1;
		LDST(currentState);
	}

	memaddr *tmp;
	tmp = &(newPcb->p_s);
	*tmp = statep;

	if(supportp != 0){
		newPcb->p_supportStruct = supportp;
	}
	else{
		newPcb->p_supportStruct = NULL;
	}

	newPcb->p_time = 0;
	newPcb->p_semAdd = NULL;
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

	if(*semAddrP < 0){
		currentProcess->p_time += (5000 - getTIMER());
		insertBlocked(semAddrP, currentProcess);
		blockedCount++;
		scheduler();
	}
	currentState->pc_epc = currentState->pc_epc + 4;
	LDST(currentState);
}

void verhogen(int* semAddrV){
	*semAddrV += 1;
	if(*semAddrV <= 0){
		pcb_t *unblocked;
		unblocked = removeBlocked(semAddrV);
		insertProcQ(&readyQ, unblocked);
		blockedCount--;
	}
}

