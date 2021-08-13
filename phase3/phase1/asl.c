#include "asl.h"
#include "pcb.h"
#include <pandos_types.h>
#include <pandos_const.h>


semd_t *semdFree_h, *semdFree_tail;
semd_t *semd_h, *semd_tail;


/*manteniamo un valore che corrisponde alla dimensione di un semaforo, usato per inizializzare la lista dei semafori liberi*/
static uint dimSemaforo = sizeof(semd_t);

/*questa funzione inizializza tutti gli elementi della semd_table inserendoli nella lista dei semafori
  liberi alla cui testa punta semdFree_h e alla cui coda punta semdFree_tail. Notamo che i semfaori vengono
  inseriti nella coda e mentre vengono deallocato dalla testa*/
void initASL(){
    /*allochiamo staticamente i semafori usando un array*/
    static semd_t semd_table[MAXPROC];

    //vengono inizializzati i puntatori alla testa e alla coda della lista dei semafori liberi
    semdFree_h = semdFree_tail = semd_table;
    semdFree_h->s_procQ = NULL;
    semdFree_h->s_semAdd = NULL;

    //viene effettuato un ciclo dove vengono inseriti tutti i semafori utilizando una funzion ausiliaria
    for(int i = 1; i < MAXPROC; i++){
        freeSem((semd_t *)((uint)semdFree_h + dimSemaforo * i));
    }

    //infine si inizializzano a null l'ultimo elemnto della lista dei semafori liberi a anche i puntatori
    //alla lista dei semafori attivi
    semdFree_tail->s_next = semd_h = semd_tail = NULL;

}

/*questa funzione si occupa di inserire il pcb puntato da p nella coda dei pcb associato al semaforo a cui e' assegmato
  il valore passato per paramtro semAdd*/
int insertBlocked(int *semAdd, pcb_t *p){
    semd_t *semPtr;

    //usando una variabile intermedia e una funzione ausiliaria, viene verificato se è presente nella lista dei semafori
    //attivi il semaforo a cui è associato il valore in semAdd. Se viene trovato, esso sarà puntato da semPtr
    if(((semPtr = isInserted(semAdd)) != NULL)){
        //se viene trovato, su invoca la funzione sui pcb insertProcQ, che permette si inserire in fondo alla coda associata a
        //semPtr il pcb puntato da p
        insertProcQ(&(semPtr->s_procQ), p);
        //fatto ciò, si assegna il numero del semaforo al campo p_semAdd del pcb appena inserito
        p->p_semAdd = semAdd;

    }
    //se il semaforo a cui è associato semAdd non è presente tra i semafori attivi, se ne estrae uno dai semafori liberi usando
    //una funzione ausiliaria, gli si assegma il valore semAdd e si inizializza la sua coda dei processi inserendo il pcb puntato da p
    else if((semPtr = allocASL(semAdd)) != NULL){
        //si inizializza la lista dei processi associati a semPtr usando una funzione per la manipolazione delle code di pcb
        semPtr->s_procQ = mkEmptyProcQ();
        //si inizializza la coda usando una funzione ausiliaria dei pcb
        initTail((memaddr *)&(semPtr->s_procQ), p);
        //fatto ciò, si assegna il numero del semaforo al campo p_semAdd del pcb appena inserito
        p->p_semAdd = semAdd;
    }
    
    //infine si ritorna un booleano, che è tru se semPtr è rimastp NULL dopo l'assegnamento di entrambe le funzioni ausiliarie
    //cioè nella lista dei semafori attivi non è presente il semaforo cercato e la lista dei semafori liberi è vuoto.
    //in tutti gli altri casi è falso
    return(semPtr == NULL);
}

/*questa funzione rimuove il primo pcb dalla coda dei pcb associato al semaforo identificato con il parametro passato*/
pcb_t* removeBlocked(int *semAdd){
    //usiamo una variabile ausiliaria per verificare che sia effettivamente inserito un semaforo associato al parametro passato
    //verifichiamo inoltre che la sua coda dei processi sia vuota o meno
    semd_t *semPtr;
    if((semPtr = isInserted(semAdd)) == NULL || semPtr->s_procQ == NULL){
        return NULL;
    }

    //se è presente, usiamo un'altra variabile ausiliaria per estrarre il pcb da restituire usando una funzione per la manipolazione
    //delle code di pcb
    pcb_t *pcbPtr;
    pcbPtr = removeProcQ(&(semPtr->s_procQ));

    //verifichiamo se, rimuovendo il pcb, abbiamo svuotato la coda dei pcbel semaforo
    if(semPtr->s_procQ == NULL){
        //in questo caso, usiamo una funzione ausiliaria per rimuovere il semaforo vuoto dalla lista dei semafori attivi
        removeSem(semPtr);
          
    }

    //infine si ritorna il pcb estratto
    return pcbPtr;
}

/*questa funzione estrae il pcb passato per argomento dalla coda dei pcb a cui è associato, il quale è individuato usando il semaforo
  a cui p è associato. Notiamo che p può trovarsi in qualunque posizione dentro la coda di pcb*/
pcb_t* outBlocked(pcb_t *p){
    //usiamo una variabile ausiliaria per verificare che il semaforo associato a p sia presente nella lista dei semafori attivi
    //e che il pcb passato sia effettivamente associato ad una coda di pcb di un semaforo attivo
    semd_t *semPtr;
    if((semPtr = isInserted(p->p_semAdd)) == NULL || p->p_semAdd == 0 || p->p_semAdd == NULL){
        return NULL;
    }

    //se le verifiche sono andate a buon fine, usiamo una variabile ausiliaria per estrarre il pcb dalla coda dei pcb attraverso
    //una funzone di manipolazione delle code di pcb
    pcb_t *pcbPtr;
    pcbPtr = outProcQ(&(semPtr->s_procQ), p);

    //infine verifichiamo se il il semaforo è stato svuotato da pcb associati, nel qual caso rimuoviamo il semaforo dalla lista dei
    //semafori attivi
    if(semPtr->s_procQ == NULL){
        removeSem(semPtr);
    }

    //alla fine si restituisce un puntatore al pcb estratto
    return pcbPtr;
}

/*questa funzione restituisce un puntatore alla testa della coda di pcb associato al semaforo a cui è assegnato il valore passato per
  argomento nel suo campo s_semAdd*/
pcb_t* headBlocked(int *semAdd){
    //verifichiamo innanzitutto che il semaforo cercato è presente nella lista dei semafori attivi e che la coda dei processi associato
    //sia vuota o meno
    semd_t *semPtr;
    if((semPtr = isInserted(semAdd)) == NULL || emptyProcQ(semPtr->s_procQ)){
        return NULL;
    }

    //se le verifiche sono andate a buon fine, usiamo una variabile aausiliaria e una funzione di manipolazione delle code di pcb
    //per estrarre un puntatore all'ultimo elemento della coda dei pcb associato al semaforo cercato
    pcb_t *pcbPtr;
    pcbPtr = headProcQ(&(semPtr->s_procQ));

    //poiché le code di pcb sono circolarmente puntati e la funzione usata estrae il pcb in fondo alla coda, si restituisce il pcb
    //successivo, che corrisponde a quello in testa
    return pcbPtr->p_next;
}

/*------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*FUNZIONI AUSILIARIE PER GESTIRE I SEMAOFORI*/
/*-------------------------------------------------------------------------------------------------------------------------------------------------------------*/


/*questa è una funzione ausiliaria che permette di inserire il semaforo puntato dall'argomento nella lista dei semafori
  liberi, inizializzando anche i loro campi*/
void freeSem(semd_t *sem){
    //si verifica prima se la lista dei semafori liberi è vuota o meno
    if(semdFree_h == NULL){
        //se è vuota si iniziializzano i puntatori alla testa e alla cosa assieme
        semdFree_h = semdFree_tail = sem;
    }
    else{
        //altrimenti, se la lista non è vuota, si inserisce il semaforo in coda
        semdFree_tail->s_next = sem;
        semdFree_tail = sem;
    }
    //infine ne si neutralizzano i campi
    sem->s_next = NULL;
    sem->s_procQ = NULL;
    sem->s_semAdd = NULL;
    
}



/*funzione ausiliaria per verificare se nella lista dei semafori attivi è presente un semaforo a cui è associato il valore
  passto per parametro semAdd. Ritorna NULL se non vien trovato o un puntatore al semaforo se viene trovato*/
semd_t *isInserted(int *semAdd){
    //inizializziamo una variail ausiliaria chiamata parser che serve per scandire la lista dei semafori attivi
    semd_t *parser;
    parser = semd_h;
    
    //scansionimo sequenzialmente la lista dei semafori attivi, e se viene trovato un semaforo con valore semAdd, viene subito
    //restituito un puntatore ad esso
    while(parser != NULL){
        if((parser->s_semAdd) == semAdd){
            return parser;
        }
        parser = parser->s_next;
    }
    //se l'esecuzione è arrivata qui, allora non è presente il semaforo ricercato e quindi si ritorna NULL
    return NULL;
}

/*funzione ausiliaria per allocare un semaforo dalla lista dei semafori liberi in quella dei semafori attivi
  Ritorna NULL se non ci sono semafori liberi o un puntatore al semaforo se ce n'e' una libera*/
semd_t *allocASL(int *semAdd){
    //innanzitutto verifichiamo se la lista dei semafori liberi ha effettivamente dei semafori liberi
    if(semdFree_h == NULL){
        //se non viene trovato un semaforo libero, ritorniamo NULL
        return NULL;
    }
    else{
        //se ha sei semafoti liberi, ne estraiamo una dalla testa e gli assegnamo il valore passato per parametro semAdd
        //nel suo campo s_semAdd
        semd_t *newSem;
        newSem = semdFree_h;
        newSem->s_semAdd = semAdd;

        //corrggiamo i puntatori alla testa e alla coda della lista dei semafori liberi verificando se, estraendo questo
        //semaofor, abbiamo completamente svuotato la lista
        if(semdFree_h->s_next == NULL){
            semdFree_h = semdFree_tail = NULL;
        }
        else{
            semdFree_h = semdFree_h->s_next;
        }

        //usiamo una funzione ausiliaria per inserire il semaforo estratto nella posizione corretta, mantenendo un ordine
        //ascendente tra i diversi semaforo usando come criterio il loro campo s_semAdd
        insertSem(newSem);
        //infine ritorniamo un puntatore al semaforo appena estratto
        return newSem;
    }

}

/*questa funzione individua la posizione corretta all'interno della lista dei semafori attivi dove inserire il semaforo passato per
  parametro newSem, in modo che i semafori siano ordinato in modo ascendente in abse al campo s_semAdd*/
void insertSem(semd_t *newSem){
    //verifichiamo innanzitutto se la lista è vuota o meno. Se è vuota viene inizializzato la lista dei de semafori attivi inserendo
    //il semaforo passato per parametro facendo punatare sia il puntatore alla testa che quello alla coda a newSem
    if(semd_h == NULL){
        semd_h = semd_tail = newSem;
        newSem->s_next = NULL;
    }
    else{
        //se la lista non è vuota, vengono verificati i casi limite, cioè il caso in cui il campo s_semAdd di newSem
        //è più piccolo di qualunque altro semaforo nella lista, o il caso in cui  è il più grande, cioè il caso in cui s_semAdd di
        //newSem è più piccolo del semaforo in testa, quindi deve diventare la nuova testa, o se è più grande del semaforo in fondo
        //alla coda, quindi newSem deve diventare la nuova coda
        if(semd_h->s_semAdd > newSem->s_semAdd){
            newSem->s_next = semd_h;
            semd_h = newSem;
        }
        else if(semd_tail->s_semAdd < newSem->s_semAdd){
            semd_tail->s_next = newSem;
            newSem->s_next = NULL;
            semd_tail = newSem;
        }

        //se i casi limite non sono verificati, allora dichiariamo una variabile ausiliaria per controllare sequenzialmente tutti
        //i semafori nella lista dei semafori attivi, e fermiamo il ciclo puntando al semaforo che diventerà il precedente del nuovo semaforo
        else{
            semd_t *scanner;
            scanner = semd_h;
            while(scanner->s_next->s_semAdd < newSem->s_semAdd){
                scanner = scanner->s_next;
            }
            //individuata la posizione, correggiamo i puntatori in modo da inserire il nuovo semaforo
            newSem->s_next = scanner->s_next;
            scanner->s_next = newSem;
        }
       
        
    }
    
}



/*questa funzione rimuove il semaforo passato come argomento dalla lista dei semafori attivi e lo restituisce alla lista
  dei semafori liberi*/
void removeSem(semd_t *semDel){
    //verrifichiamo innanzitutto il caso limite in cui il semaforo da eliminare si trova alla testa dei semafori attivi
    if(semDel == semd_h){
        semd_h = semd_h->s_next;
    }
    else{
        //altrimenti si usa una variabile ausiliaria per individuare la posizione del semaforo precedente a quello da eliminare
        semd_t *scanner;
        scanner = semd_h;
        while(scanner->s_next != semDel){
            scanner = scanner->s_next;
        }
        //si corregge il puntatore del semaforo precedente a quello da eliminare
        scanner->s_next = semDel->s_next;
        
        //si verifica se il semaforo da eliminare è l'ultimo della coda, e in quel caso si corregge il puntatore alla coda della
        //lista dei semafori attivi
        if(semd_tail == semDel){
            semd_tail = scanner;
        }
    }
    //infine, si reinserisce il semaforo alla lista dei semafori liberi usando una funzione ausiliaria
    freeSem(semDel);
}

