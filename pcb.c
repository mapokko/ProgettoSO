#include "pcb.h"
#include <pandos_types.h>
#include <pandos_const.h>

#define MAXPROC 20

static pcb_t pcbFree_table[MAXPROC];
static pcb_t *pcbFree_h;
static uint campo = sizeof(pcb_PTR);
static uint processo = sizeof(pcb_t);


//TUTTE LE VARIABILI SONO STATIC PER MOTIVI DI DEBUG, PER FAVORE DICHIARATE LE VARIABILI COME STATIC!!
//al termine del progetto li togliamo tutti

void setPointers(memaddr *process,pcb_t *setNext, pcb_t *setPrev){
	pcb_t *processPtr;
	processPtr = process;

	processPtr->p_next = setNext;
	processPtr->p_prev = setPrev;
}

void initPcbs(){
	//variabile che serve per sapere via via gli indirizzo di mem
	//dove cominciano i pcb
	static memaddr *contatore;

	//inizializzo contatore in modo che punti al primo pcb dell'array
	contatore = pcbFree_table;

	initHead(contatore);

	//variabile che serve per puntare via via tutti i pcb dell'array
	static pcb_t *pcbPointer;

	//effettuo un ciclo che fa uno scan di tutti i pcb dentro il vettore e 
	//li inserisce nella lista alla cui testa punta pcbFree_h
	for(int i = 1; i < MAXPROC; i++){
		contatore  = (uint) contatore + processo; 
		pcbPointer = contatore;
		freePcb(pcbPointer);
	}
	

}

void initHead(memaddr *pointer){

	pcbFree_h = pointer;
	setPointers(pcbFree_h, pcbFree_h, pcbFree_h);
}

void freePcb(pcb_t *p){
	static memaddr *newPcb;
	newPcb = p;

	if (pcbFree_h == NULL){
		initHead(newPcb);
		return;
	}

	setPointers(newPcb, pcbFree_h->p_next, pcbFree_h);
	setPointers(pcbFree_h, p, pcbFree_h->p_prev);
	setPointers(p->p_next, p->p_next->p_next, p);
	pcbFree_h = p;
}

pcb_t *allocPcb(){
	if (emptyProcQ(pcbFree_h)){
		return NULL;
	}

	static pcb_t *toReturn;
	toReturn = pcbFree_h;
	
	if(pcbFree_h->p_next == pcbFree_h){
		pcbFree_h = NULL;
	}
	else{
		pcbFree_h = pcbFree_h->p_prev;
		setPointers(pcbFree_h, pcbFree_h->p_next->p_next, pcbFree_h->p_prev);
		setPointers(pcbFree_h->p_next, pcbFree_h->p_next->p_next, pcbFree_h);
	}

	setValues(toReturn);
	return toReturn;
}

void setValues(pcb_t *pointer){
	pointer->p_prev = 0;
	pointer->p_next = 0;
	pointer->p_prnt = 0;
	pointer->p_child = 0;
	pointer->p_next_sib = 0;
	pointer->p_prev_sib = 0;
}



//WORK IN PROGRESS: teoricamente dovrebbe essere tutto qui, ma non ho ancora testato
pcb_t *mkEmptyProcQ(){
	pcb_t *toReturn;

	toReturn = NULL;

	return toReturn;
}

//WORK IN PROGESS: completato ma non so se funziona
int emptyProcQ(pcb_t *tp){
	if(tp == NULL){
		return 1;
	}
	else{
		return 0;
	}
}


void insertProcQ(pcb_t **tp, pcb_t *p){
	static memaddr *tailAddr;
	tailAddr = tp;

	if(emptyProcQ(*tp)){
		initTail(tailAddr, p);
		return;
	}

	static pcb_t *tail; 
	tail = *tailAddr;

	setPointers(p, tail, tail->p_prev);
	setPointers(tail, tail->p_next, p);
	setPointers(p->p_prev, p, p->p_prev->p_prev);

}

void  initTail(memaddr *tailAddress, pcb_t *p){
	*tailAddress = p;
	setPointers(p, p, p);
}

pcb_t *headProcQ(pcb_t **tp){
	static memaddr *tailAddress;
	tailAddress = tp;
	if (*tailAddress == NULL){
		return NULL;
	}

	static pcb_t *tail;
	tail = *tailAddress;

	return tail->p_prev;
}

pcb_t* removeProcQ(pcb_t **tp){
	if(emptyProcQ(*tp)){
		return NULL;
	}

	static memaddr *tailAddress;
	static pcb_t *tailPtr;
	tailAddress = tp;
	tailPtr = *tailAddress;

	if(tailPtr == tailPtr->p_next){
		*tailAddress = NULL;
	}
	else{
		setPointers(tailPtr->p_next, tailPtr->p_next->p_next, tailPtr->p_prev);
		setPointers(tailPtr->p_prev, tailPtr->p_next, tailPtr->p_prev->p_prev);
		*tp = tailPtr->p_next;
	}

	return tailPtr;
}

int isPresent(pcb_t *tail, pcb_t *p){
	static pcb_t *scanner;
	scanner = tail;
	do{
		if(scanner == p){
			return 1;
		}
		scanner = scanner->p_next;
	}while(scanner != tail);

	return 0;
}

pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
	if(!isPresent(*tp, p)){
		return NULL;
	}

	
	setPointers(p->p_next, p->p_next->p_next, p->p_prev);
	setPointers(p->p_prev, p->p_next, p->p_prev->p_prev);

	if(p == *tp){
		*tp = p->p_next;
	}
	
	return p;
}