#include "asl.h"
#include "pcb.h"
#include <pandos_types.h>
#include <pandos_const.h>

static semd_t semd_table[MAXPROC];
static semd_t *semdFree_h;
static semd_t *semd_h, *semd_tail;
static uint dimSemaforo = sizeof(semd_t);

/*funzione richiesta, funziona ma è da ottimizzare*/
void initASL(){
    static memaddr *semdAddr;
    semdAddr = semd_table;
    semdFree_h = semd_table;

    static semd_t *parser;
    parser = semdAddr;

    for(int i = 1; i < MAXPROC; i++){
        semdAddr = (uint)semdAddr + dimSemaforo;
        parser->s_next = semdAddr;
        parser = parser->s_next;
    }
    parser->s_next = NULL;
    semd_h = NULL;
    semd_tail = NULL;

}

/*incompleta e non ancora testata*/
int insertBlocked(int *semAdd, pcb_t *p){
    static semd_t *semPtr;

    if((semPtr = isInserted(semAdd) != NULL)){
        insertProcQ(&(semPtr->s_procQ), p);

    }
    else if((semPtr = allocASL(semAdd)) != NULL){
        semPtr->s_procQ = mkEmptyProcQ();
        initTail(semPtr->s_procQ, p);
    }

    return(semPtr == NULL);
}

/*funzione ausiliaria per insertBlocked*/
int isInserted(int *semAdd){
    static semd_t *parser;
    parser = semd_h;

    while(parser != NULL){
        if(parser->s_semAdd == semAdd){
            return parser;
        }
        parser = parser->s_next;
    }

    return NULL;
}

/*funzione ausiliaria per insertBlocked*/
int allocASL(int *semAdd){
    if(semdFree_h->s_next == NULL){
        return NULL;
    }
    else{
        static semd_t *newTail;
        newTail = semdFree_h;
        semdFree_h = semdFree_h->s_next;

        if(semd_h == NULL){
            semd_h = semd_tail = newTail;
            newTail->s_next = NULL;
        }
        else{
            semd_tail->s_next = newTail;
            semd_tail = newTail;
        }

        newTail->s_semAdd = semAdd;
        return newTail;
    }

}