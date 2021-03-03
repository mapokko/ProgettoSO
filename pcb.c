#include "pcb.h"
#include <pandos_types.h>
#include <pandos_const.h>

#define MAXPROC 20

static pcb_t pcbFree_table[MAXPROC];
static pcb_t *pcbFree_h;
static unsigned int campo = sizeof(pcb_PTR);
static unsigned int processo = sizeof(pcb_t)/4;

/*void initPcbs(){
	static memaddr *puntatore, *next, *prev;

	puntatore = pcbFree_table;
	pcbFree_h = puntatore;
	next = puntatore + campo;
	prev = next + campo;
	*next = *prev = pcbFree_h;

	static memaddr *test;
	test = pcbFree_table + processo * 20;

	static pcb_t *tmp;

	for(int i = 1; i < MAXPROC; i++){
		puntatore = puntatore + processo;
		tmp = puntatore;
		freePcb(tmp);
	}

	return;
}*/

/*void freePcb(pcb_t *new){
	static pcb_t *control;

	static memaddr *p ,*head;
	head = pcbFree_h;
	p = new;

	static memaddr *next2, *prev2, *headnext;
	headnext = head + campo;
	control = *headnext;

	next2 = p + campo;
	prev2 = next2 + campo;

	*next2 = control;

	*prev2 = pcbFree_h;

	p = control;
	prev2 = p + (campo * 2);
	*prev2 = new;

	p = pcbFree_h;
	next2 = p + campo;
	*next2 = new;

	pcbFree_h = new;
}*/

//TUTTE LE VARIABILI SONO STATIC PER MOTIVI DI DEBUG, PER FAVORE DICHIARATE LE VARIABILI COME STATIC!!
//al termine del progetto li togliamo tutti

void initPcbs(){
	initHead();
	
	static memaddr *contatore;

	contatore = pcbFree_table;
	contatore  = contatore + processo;

	static pcb_t *pcbPointer;

	pcbPointer = contatore;

	freePcb(pcbPointer);

}

//inizializza il puntatore alla testa della lista circolare e setta anche 
//i campi next e prev del primo elemento del array pcbFree_table in modo che puntino al pcb stesso
void initHead(){
	//variabili di indirizzi di memoria
	static memaddr *pointToFirst, *next, *prev;

	//inizializzo pointToFirst con l'indirizo di memoria dove comincia il primo pcb dell'array
	pointToFirst = pcbFree_table;

	//inizializzo il puntatore del alla tesat della lista, dandogli appunto l'indirizzo della testa della lista
	pcbFree_h = pointToFirst;

	//setto i puntatori next e prev con l'indirizzo di memoria dei campi p_next e p_prev
	//così che quando li dereferenzio, poso cambiare il valore nella cella di memoria
	//puntata da loro
	next = pointToFirst;
	prev = pointToFirst + campo;

	//infine qua avviene la dereferenziazzione, assegnano i puntatoti p_next e p_prev del primo pcb
	//in modo che puntino al pcb stesso
	*next = *prev = pcbFree_h;

	//N.B.: ho visto che quando si dereferenziano i dei puntatori interni della struttura, la right-side-value
	//deve sempre essere di type pcb_t *
}

void freePcb(pcb_t *p){
	static memaddr *newPcb, *next, *prev, *tailAddress;
	static pcb_t *tail;

	tailAddress = pcbFree_h;
	tail = *tailAddress;

	newPcb = p;
	next = newPcb;
	prev = newPcb + campo;

	*next = tail;
	*prev = pcbFree_h;

	*newPcb = p;
	tailAddress = tail + campo;
	*tailAddress = p;

}