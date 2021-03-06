#include "pcb.h"
#include <pandos_types.h>
#include <pandos_const.h>

#define MAXPROC 20

static pcb_t pcbFree_table[MAXPROC];
static pcb_t *pcbFree_h;

/*processo è il valore usato per indicare la dimensione occupata da un pcb, ovvero la dimensione di una singola cella
  dentro pcbFree_table. E' usato per calcolare l'offset per raggiungere i diversi inidirizzi in ci cominciano le
  celle, ovvero i pcb di pcbFree_table*/
static uint processo = sizeof(pcb_t);


//TUTTE LE VARIABILI SONO STATIC PER MOTIVI DI DEBUG, PER FAVORE DICHIARATE LE VARIABILI COME STATIC!!
//al termine del progetto li togliamo tutti

/*inizializza la lista dei pcb liberi aggiungendo tutti i pcb contenuti in pcbFree_table e alla testa
  di questa lista punta pcbFree_h. Notiamo che la lista dei pcb liberi è una lista monodirezionale collegata
  con dal campo p_next mentre il campo p_prev è settato a 0. La lista termina con l'ultimo pcb che punta a NULL*/
void initPcbs(){
	initHead(pcbFree_table);

	//pcbPoitner è un puntatore a pcb che serve per scorrere via via l'array di pcbFree_table in modo da puntare
	//i processi in pcbFree_table
	static pcb_t *pcbPointer;

	for(int i = 1; i < MAXPROC; i++){
		//in particolare, usiamo l'indirizzo di inizio da cui cominciano le celle di pcbFree_table e usiamo un offset
		//via via crescente per identificare l'indirizzo dove cominciano le varie celle
		pcbPointer = (uint) pcbFree_table + processo * i;
		//viene usata la funzione freePcb per inserire i vari pcb puntati da pcbPointer all'interno della lista di pcb libero
		//la cui testa è puntata da pcbFree_h
		freePcb(pcbPointer);
	}
}

/*questa fuzione prende il pcb puntato da p e lo inserisce alla testa della lista dei pcb liberi, aggiornando poi pcbFree_h*/
void freePcb(pcb_t *p){
	//viene prima di tutto verificato se la lista dei pcb è completamente vuota, in tal caso si riutilizza la funzione per 
	//inizializzare la testa
	if (pcbFree_h == NULL){
		initHead(p);
		return;
	}
	//viene chiamata la fuzione setPointers che si occupa di impostare i puntatori di di p, mettendo nel suo campo p_next
	//l'indirizzo del pcb puntato al momento dal puntatore alla tesat della lista
	setPointers(p, pcbFree_h, 0);
	//viene poi aggiornata la testa dei pcb liberi
	pcbFree_h = p;
}

/*qusta funzione di occupa di estrarre un pcb dalla lista dei pcb liberi e resituirla. Se la lista è vuota restituisce NULL*/
pcb_t *allocPcb(){
	//viene prima verificata se la lista è vuota o meno. Nel primo caso viene subito restituito NULL
	if (emptyProcQ(pcbFree_h)){
		return NULL;
	}
	//viene utilizata la variabile toReturn per mantere un puntatore al pcb che deve essere restituito
	static pcb_t *toReturn;
	toReturn = pcbFree_h;
	
	//viene aggiornato il puntatore alla testa della lista dei pcb liberi
	pcbFree_h = pcbFree_h->p_next;

	//vengono impostato i valori del pcb da restituire
	setValues(toReturn);
	return toReturn;
}

/*questa funzione si occupa di tonare un valor da impostare il puntatore  ad una nuova coda di processi che è al momento vuota,
  in particolare vediamo che il puntatore ad una coda dei processi viene inizializzata con NULL*/
pcb_t *mkEmptyProcQ(){
	return NULL;
}

/*questa funzione verifica se il puntatore a pcb tp punta ad un pcb o non punta a nulla*/
int emptyProcQ(pcb_t *tp){
	if(tp == NULL){
		return 1;
	}
	else{
		return 0;
	}
}

/*questa funzione si occupa di inserire il pcb puntato da p, nella lista di processi il cui puntatore alla coda è puntato
da tp. In particolare si occupa di inizializzare la coda dei processi nel casoil puntatore puntato da tp non punti a nulla.*/
void insertProcQ(pcb_t **tp, pcb_t *p){
	//viene verificato se il puntatore puntato da tp punta effettivamente a qualcosa o se deve essere inizializzata
	if(emptyProcQ(*tp)){
		//questa è una funzione che inizalizza il puntatore a una coda di processi
		initTail(tp, p);
		return;
	}

	//viene usata una variabile ausiliaria per manipoalare i campi del pcb puntato dal puntatore alla coda,
	//che è a sua volta puntato da tp. Per questo motivo a tail si assegna *tp e non tp e basta
	static pcb_t *tail; 
	tail = *tp;

	//queste tre invocazioni impostano i campi p_next e p_prev rispettivamente di: il nuovo pcb puntato da p, il pcb
	//puntato dal puntatore alla coda e infine il pcb che si trova alla testa della coda, il cui campo p_next deve essere
	//aggiornata puntando al nuovo pcb in testa
	setPointers(p, tail, tail->p_prev);
	setPointers(tail, tail->p_next, p);
	setPointers(p->p_prev, p, p->p_prev->p_prev);
}

/*questa funzione prende un puntatore al puntatore alla coda di una coda dei processi e si occupa di ritornare un puntatore
  al pcb che si trova in testa alla cosa dei processi*/
pcb_t *headProcQ(pcb_t **tp){
	//si verifica innazitutto se il puntatore alla cosa sta effettivamente puntando alla coda
	if (*tp == NULL){
		return NULL;
	}
	//si usa una variabile intermedia per accedera i campi interni del pcb in fondo alla coda e si utilizza dil cuo campo
	//p_prev per risalire alla testa
	static pcb_t *tail;
	tail = *tp;
	return tail->p_prev;
}

/*questa funzione si occupa di rimuovere il pcb che si trova in fondo alla coda della coda dei processi puntata dal
  puntatore alla coda puntato da tp*/
pcb_t* removeProcQ(pcb_t **tp){
	//viene prima verificato se è possibile rimuovere un pcb dalla coda, cioè se la coda è vuota o meno
	if(emptyProcQ(*tp)){
		return NULL;
	}

	//definiamo una variabile ausiliaria per manipolare i campo interni del puntatore passato ttraverso tp
	static pcb_t *tailPtr;
	tailPtr = *tp;

	//nel caso il puntatore contenga un solo elemento, facciamo in modo che in puntatore alla coda di pcb
	//punti a NULL poiché da quinon contiene più elementi
	if(tailPtr == tailPtr->p_next){
		*tp = NULL;
	}
	//se invece contiene elementi, facciamo aggioraniamo i puntatori del pcb in testa alla coda e aggioraniamo l'elemento
	//in fondo alla coda
	else{
		setPointers(tailPtr->p_next, tailPtr->p_next->p_next, tailPtr->p_prev);
		setPointers(tailPtr->p_prev, tailPtr->p_next, tailPtr->p_prev->p_prev);
		*tp = tailPtr->p_next;
	}

	//infine ritorniao il puntatore allìelemtno appna estratto dalla coda
	return tailPtr;
}

/*questa funzione prmette di estrarre dalla cosa di pcb puntato dal puntatore contenuto in tp in modo da estarre esattamente il pcb
  puntato da p, che si può trovare in qualunque posizione dentro la cosa. In particolare, se p non è presente nella cosa contenuta in tp,
allora ritoriamo NULL;*/
pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
	//verifichiamo se p è presente nella coda puntata da tp usanod una funzione ausiliaria
	if(!isPresent(*tp, p)){
		return NULL;
	}

	//se p è contenuto nella coda passata attraverso tp, allora aggiunstiamo i puntatori dei pcb prima e dopo p
	setPointers(p->p_next, p->p_next->p_next, p->p_prev);
	setPointers(p->p_prev, p->p_next, p->p_prev->p_prev);

	//in particolare, se p punta al pcb che si trova in fondo alla cosa, aggiorniamo anche il puntatore alla coda associato
	//alla coda passata
	if(p == *tp){
		*tp = p->p_next;
	}
	
	return p;
}

/*DA QUI SOTTO COMINCIANO FUNZIONI AUSILIARE PER IL FUNZIONAMENTO DI QUELLE RICHIESTE.
  FEEL FREE TO USE THEM AS YOU LIKE!*/

/*questa funzione si occupa di impostare i campi p_next e p_rev del pcb il cui indirizzo è contenuto in ProcessPtr
  il pcb setNext diventa quello puntato dal campo p_next e il pcb p_prev punta al pcb puntato dal campo setPrev*/
void setPointers(pcb_t *processPtr,pcb_t *setNext, pcb_t *setPrev){
	processPtr->p_next = setNext;
	processPtr->p_prev = setPrev;
}

/*questa funzione si occupa di impostare il puntatore pcbFree_h quando la lista dei pcb liberi è completamente vuota,
  ovvero pcbFree_h sta puntando a NULL*/
void initHead(memaddr *pointer){
	pcbFree_h = pointer;
	setPointers(pcbFree_h, NULL, 0);
}

/*questa funzione si occupa di impostare tutti i campi dei puntatori dentro al pcb a 0. Osserviamo che imposta anche il
  che serve poi per mantnere l'indirisso del semaforo ad esso associato*/
void setValues(pcb_t *pointer){
	pointer->p_prev = 0;
	pointer->p_next = 0;
	pointer->p_prnt = 0;
	pointer->p_child = 0;
	pointer->p_next_sib = 0;
	pointer->p_prev_sib = 0;
	pointer->p_semAdd = 0;
}

/*questa funzione si occupa di inizializzare la coda dei processi la cui coda punta tailAddress, con il solo pcb puntato da p.
  In particolare il pcb puntato da p viene inizializzato con con i campi p_next e p_prev che puntano a se stesso*/
void  initTail(memaddr *tailAddress, pcb_t *p){
	*tailAddress = p;
	setPointers(p, p, p);
}

/*questa funzione ausiliaria prende in input un puntatore a coda i pcb e un puntatroe a un certo pcb, effettua una scansione
  lineare della coda finché non trova p. Se non lo trova restituisce 0, se lo trova restituisce l'indirizzo in cui si trova*/
int isPresent(pcb_t *tail, pcb_t *p){
	//si usa una variabile ausiliaria per confrontare ogni elemento della lista
	static pcb_t *scanner;
	scanner = tail;
	
	//si confronta p con scanner finché scanner non ritorna a puntare su tail. Osserviamo che se la coda puntata da tail contiene
	//un solo elemento, il ciclo do while viene eseguito almeno una sola volta, nel caso quel pcb fosse quello ricercato
	do{
		if(scanner == p){
			return 1;
		}
		scanner = scanner->p_next;
	}while(scanner != tail);

	return 0;
}

