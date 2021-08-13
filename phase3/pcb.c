#include "pcb.h"
#include <pandos_types.h>
#include <pandos_const.h>

/*processo è il valore usato per indicare la dimensione occupata da un pcb, ovvero la dimensione di una singola cella
  dentro pcbFree_table. E' usato per calcolare l'offset per raggiungere i diversi inidirizzi in ci cominciano le
  celle, ovvero i pcb di pcbFree_table*/
static uint processo = sizeof(pcb_t);
pcb_t *pcbFree_h;

/*inizializza la lista dei pcb liberi aggiungendo tutti i pcb contenuti in pcbFree_table e alla testa
  di questa lista punta pcbFree_h. Notiamo che la lista dei pcb liberi è una lista monodirezionale collegata
  con dal campo p_next mentre il campo p_prev è settato a 0. La lista termina con l'ultimo pcb che punta a NULL*/
void initPcbs(){
	//avviene qui la dichiarazione dlell'array che assegna o spazio di memoria effettiov per tutti i successivi
	//pcb che verranno manipolato nel corso del resto del programma
	static pcb_t pcbFree_table[MAXPROC];

	initHead((memaddr *)pcbFree_table);

	//pcbPoitner è un puntatore a pcb che serve per scorrere via via l'array di pcbFree_table in modo da puntare
	//i processi in pcbFree_table
	pcb_t *pcbPointer;

	for(int i = 1; i < MAXPROC; i++){
		//in particolare, usiamo l'indirizzo di inizio da cui cominciano le celle di pcbFree_table e usiamo un offset
		//via via crescente per identificare l'indirizzo dove cominciano le varie celle
		pcbPointer = (pcb_t *)(pcbFree_table + processo * i);
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
		initHead((memaddr *)p);
		return;
	}
	//si resettano tutti i campi del pcb prima di reinserirlo nella lista dei pcb liberi
	setValues(p);

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
	pcb_t *toReturn;
	toReturn = pcbFree_h;

	//viene aggiornato il puntatore alla testa della lista dei pcb liberi
	pcbFree_h = pcbFree_h->p_next;

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
		initTail((memaddr *)tp, p);
		return;
	}

	//viene usata una variabile ausiliaria per manipoalare i campi del pcb puntato dal puntatore alla coda,
	//che è a sua volta puntato da tp. Per questo motivo a tail si assegna *tp e non tp e basta
	pcb_t *tail;
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
	if (*tp == NULL || *tp == 0){
		return NULL;
	}
	//si usa una variabile intermedia per accedera i campi interni del pcb in fondo alla coda e si utilizza dil cuo campo
	//p_prev per risalire alla testa
	pcb_t *tail;
	tail = *tp;
	return tail->p_prev;
}

/*questa funzione si occupa di rimuovere il pcb che si trova in testa alla coda della coda dei processi puntata dal
  puntatore alla coda puntato da tp*/
pcb_t* removeProcQ(pcb_t **tp){
	//semplicemente, la rimozione del pcb che si trova nella codaa da più tempo è un caso particolare di outProcQ, dove il pcb
	//da rimuovere è quello puntatore dal puntatore alla coda
	return outProcQ(tp, *tp);
}

/*questa funzione permette di estrarre dalla coda di pcb puntato dal puntatore contenuto in tp in modo da estarre esattamente il pcb
  puntato da p, che si può trovare in qualunque posizione dentro la coda. In particolare, se p non è presente nella cosa contenuta in tp,
  allora ritoriamo NULL;*/
pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
	//verifichiamo se p è presente nella coda puntata da tp usanod una funzione ausiliaria
	if(emptyProcQ(*tp) || !isPresent(*tp, p)){
		return NULL;
	}

	//se p è contenuto nella coda passata attraverso tp, allora aggiunstiamo i puntatori dei pcb prima e dopo p
	setPointers(p->p_next, p->p_next->p_next, p->p_prev);
	setPointers(p->p_prev, p->p_next, p->p_prev->p_prev);

	//verifichiamo se il pcb che stiamo rimovendo è in testa alla cosa, in quel caso aggiorniamo anche il puntatore alla coda. Inoltre
	//se il pcb che sta venendo rimosso è l'unico presente, mettiamo il puntatore alla coda a NULL
	if (p == p->p_next){
		*tp = NULL;
	}
	else if(*tp == p){
		*tp = p->p_next;
	}


	p->p_next = p->p_prev = 0;
	return p;
}

/*questa funzione ritorna TRUE se il pcb passato per parametro non ha figli, FALSE altrimenti*/
int emptyChild(pcb_t *p){
	return (p->p_child == NULL || p->p_child == 0);
}

/*questa funzione si occupa di inserire il pcb puntato da p come ultimo figlio nella lista dei figli del pcb puntato da prnt
  in particolare si occupa anche di inizializzare il campo p_child di prnt nel caso non abbia ancora figli*/
void insertChild(pcb_t *prnt, pcb_t *p){
	//verifichiamo se prnt ha figli o meno
	if (emptyChild(prnt)){
		//se non ne ha, inizializza il campo p_child di prnt e inizializza i campi di p
		prnt->p_child = p;
		setPointerTree(p, prnt, NULL, NULL);
		return;
	}
	//si usa una variabile ausiliaria per mantenere l'ultimo figlio di prnt
	pcb_t *lastChild;

	//si usa una funzione ausiliaria che restituisce l'ultimo figlio dalla lista dei filgi di prnt
	lastChild = getLastChild(prnt);

	//si correggono i campi dell'ultimo figlio e di p in modo che diventi il nuovo ultimo figlio
	lastChild->p_next_sib = p;
	p->p_prev_sib = lastChild;
	p->p_next_sib = NULL;

	//si corregge il campo parent di p in modo che punti a prnt
	p->p_prnt = prnt;

}

/*questa funzione rimuove il primo figlio della lista dei figli del pcb puntato da p*/
pcb_t *removeChild(pcb_t *p){
	//verifichiamo se effettivamente p ha dei figli
	if(emptyChild(p)){
		p = NULL;
	}
	//se li ha, usiamo la funzione outchild poiché è un caso particolare in cui il pcb da rimuovere è il primo
	//figlio della lista
	else{
		p = outChild(p->p_child);
	}

	return p;
}

/*questa funzione si occupa di rimuove il pcb p dalla lista dei figli a cui appartiene, in particolare p può trovarsi
  in una qualunque posizione all'interno della lista dei figli*/
pcb_t *outChild(pcb_t *p){
	//verifichiamo se effettivamente p appartiene a una lista di figli
	if(p->p_prnt == NULL){
		return NULL;
	}

	//verifichiamo se p è il primo figlio della lista, e in tal caso cambiamo il primo figlio
	if(p->p_prnt->p_child == p){
		p->p_prnt->p_child = p->p_next_sib;
	}
	//se p non è il primo figlio, allora deve necessariamente avere un prev_sib, quindi correggiamo il suo campo next_sib
	else{
		p->p_prev_sib->p_next_sib = p->p_next_sib;
	}

	//infine verifichiamo se p ha un fratello successico, in tal caso correggiamo il suo campo prev_sib
	if(p->p_next_sib != NULL){
		p->p_next_sib->p_prev_sib = p->p_prev_sib;
	}


	return p;
}



/*DA QUI SOTTO COMINCIANO FUNZIONI AUSILIARE PER IL FUNZIONAMENTO DI QUELLE RICHIESTE*/
/*------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------*/



/*questa funzione si occupa di impostare il puntatore pcbFree_h quando la lista dei pcb liberi è completamente vuota,
  ovvero pcbFree_h sta puntando a NULL*/
void initHead(memaddr *pointer){
	pcbFree_h = (pcb_t *)pointer;
	setPointers(pcbFree_h, NULL, 0);
}


/*questa funzione si occupa di impostare i campi p_next e p_rev del pcb il cui indirizzo è contenuto in ProcessPtr
  il pcb setNext diventa quello puntato dal campo p_next e il pcb p_prev punta al pcb puntato dal campo setPrev*/
void setPointers(pcb_t *processPtr,pcb_t *setNext, pcb_t *setPrev){
	processPtr->p_next = setNext;
	processPtr->p_prev = setPrev;
}


/*questa funzione si occupa di impostare tutti i campi dei puntatori dentro al pcb a 0. Osserviamo che imposta anche il
  che serve poi per mantnere l'indirisso del semaforo ad esso associato*/
void setValues(pcb_t *pointer){
	pointer->p_prev = 0;
	pointer->p_next = 0;
	pointer->p_prnt = NULL;
	pointer->p_child = NULL;
	pointer->p_next_sib = NULL;
	pointer->p_prev_sib = NULL;
	pointer->p_semAdd = NULL;
	pointer->p_supportStruct = NULL;
	pointer->p_time = 0;
}

/*questa funzione si occupa di inizializzare la coda dei processi la cui coda punta tailAddress, con il solo pcb puntato da p.
  In particolare il pcb puntato da p viene inizializzato con con i campi p_next e p_prev che puntano a se stesso*/
void  initTail(memaddr *tailAddress, pcb_t *p){
	*tailAddress = (memaddr)p;
	setPointers(p, p, p);
}

/*questa funzione ausiliaria prende in input un puntatore a coda i pcb e un puntatroe a un certo pcb, effettua una scansione
  lineare della coda finché non trova p. Se non lo trova restituisce 0, se lo trova restituisce 1*/
int isPresent(pcb_t *tail, pcb_t *p){
	//si usa una variabile ausiliaria per confrontare ogni elemento della lista
	pcb_t *scanner;
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


/*questa è una funzione ausiliaria che si occupa di settare i campi parent, nextsib, prevsib del pcb passato
  come parametro*/
void setPointerTree(pcb_t *p, pcb_t *prnt, pcb_t *nextSib,pcb_t *prevSib){
	p->p_prnt = prnt;
	p->p_next_sib = nextSib;
	p->p_prev_sib = prevSib;
}


/*questa è unz funzione ausiliaria che restituisce l'ultimo filgio dalla lista dei figli del pcb passato
  come parametro*/
pcb_t* getLastChild(pcb_t *prnt){
	//usiamo una variabile ausiliaria per raggiungere l'ultimo figlio attraverso una visita sequenziale della lista dei figli
	pcb_t *child;
	//inizializziamo child come primo figlio del pcb passato per parametro
	child = prnt->p_child;

	//usiamo un ciclo while per raggiungere l'ultimo figlio e lo restituiamo
	while(child->p_next_sib != NULL || child->p_next_sib == 0){
		child = child->p_next_sib;
	}
	return child;
}
