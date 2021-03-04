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

//devo ancora commentare questa funzione
void initPcbs(){
	//chiamo la funzione initHead, che inizializza il puntatore alla testa 
	//e imposta i cmapi p_next e p_prev del primo elemento dell'array
	initHead();
	
	//variabile che serve per sapere via via gli indirizzo di mem
	//dove cominciano i pcb
	static memaddr *contatore;

	//inizializzo contatore in modo che punti al secondo pcb dell'array
	contatore = pcbFree_table;
	contatore  = (uint) contatore + processo;

	//variabile che serve per puntare via via tutti i pcb dell'array
	static pcb_t *pcbPointer;

	//effettuo un ciclo che fa uno scan di tutti i pcb dentro il vettore e 
	//li inserisce nella lista alla cui testa punta pcbFree_h
	for(int i = 1; i < MAXPROC; i++){
		pcbPointer = contatore;
		freePcb(pcbPointer);

		contatore  = (uint) contatore + processo; 
	}
	

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
	next = (uint)pointToFirst;
	prev = (uint)pointToFirst + campo;

	//infine qua avviene la dereferenziazzione, assegnano i puntatoti p_next e p_prev del primo pcb
	//in modo che puntino al pcb stesso
	*next = *prev = pcbFree_h;

	//N.B.: ho visto che quando si dereferenziano i dei puntatori interni della struttura, la right-side-value
	//deve sempre essere di type pcb_t *
}

//il pcb puntato fa p viene reinserito nella lista dei pcb liberi alla cui testa
//punta pcbFree_h
void freePcb(pcb_t *p){
	//variabili necessarie per gestire l'aggiunta del pcb puntato da *p
	static memaddr *newPcb, *next, *prev, *tailAddress;
	static pcb_t *tail;

	//al termine di queste due operazioni, tail conterrà l'indirizzo di memoria dove comincia la coda
	tailAddress = pcbFree_h;
	tail = *tailAddress;

	//queste operazioni settano next e prev con gli indirizzi dei campi p_next e p_prev
	//del pcb da inserire nella lista
	newPcb = p;
	next = (uint)newPcb;
	prev = (uint)newPcb + campo;

	//vengono settati i puntotaori dei campi del pcb da reinserire
	*next = tail;
	*prev = pcbFree_h;

	//viene settato il p_next della vecchia testa della lisat in modo che
	//punti al pcb da inserire
	next = pcbFree_h;
	*next = p;

	//analogo del campo p_prev della coda
	prev = (uint)tail + campo;
	*prev = p;

	//imposto il pcb inserito come la nuova coda
	pcbFree_h = p;
}

//WORK IN PROGRESS
pcb_t *allocPcb(){
	if(pcbFree_h == NULL){
		return NULL;
	}
	else if (isOneLeft()){
		static pcb_t *toReturn;

		toReturn = pcbFree_h;
		pcbFree_h = NULL;
		return toReturn;
	}
	else{
		pcb_t *toReturn;
		toReturn = pcbFree_h;

		static memaddr *next1, *head;

		head = toReturn;
		next1 = (uint)head;






	}
}

//WORK IN PROGRESS
int isOneLeft(){
	static memaddr *next;
	static pcb_t *compare;
	next = pcbFree_h;
	compare = *next;

	return !(compare == pcbFree_h);
}