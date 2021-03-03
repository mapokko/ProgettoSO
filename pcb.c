#include "pcb.h"
#include <pandos_types.h>
#include <pandos_const.h>

#define MAXPROC 20

static pcb_t pcbFree_table[MAXPROC];
static pcb_t *pcbFree_h;
static unsigned int campo = sizeof(pcb_PTR);
static unsigned int processo = sizeof(pcb_t);

void initPcbs(){
	static memaddr *puntatore, *next, *prev;

	puntatore = pcbFree_table;
	pcbFree_h = puntatore;
	next = puntatore + campo;
	prev = next + campo;
	*next = *prev = pcbFree_h;
//cazzofigapopo
	static pcb_t *tmp;

	for(int i = 1; i < MAXPROC; i++){
		puntatore = puntatore + processo;
		tmp = puntatore;
		freePcb(tmp);
	}

	return;
}

void freePcb(pcb_t *new){
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
}
