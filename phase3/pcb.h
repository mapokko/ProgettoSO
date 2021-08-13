#include <pandos_types.h>
#include <pandos_const.h>
#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

/*puntatore alla lista dei pcb liberi*/
pcb_t *pcbFree_h;

/*funzioni di generazione, allocazione e deallocazione dei pcb*/
extern void initPcbs();
extern void freePcb(pcb_t *p);
extern pcb_t *allocPcb();

/*funzioni di manipolazione delle code di pcb*/
extern pcb_t *mkEmptyProcQ();
extern int emptyProcQ(pcb_t *tp);
extern void insertProcQ(pcb_t **tp, pcb_t *p);
extern pcb_t *headProcQ(pcb_t **t);
extern pcb_t *removeProcQ(pcb_t **tp);
extern pcb_t *outProcQ(pcb_t **tp, pcb_t *p);

/*funzioni di gestione dei alberi di pcb*/
extern int emptyChild(pcb_t *p);
extern void insertChild(pcb_t *prnt, pcb_t *p);
extern pcb_t *outChild(pcb_t *p);
extern pcb_t *removeChild(pcb_t *p);

/*funzioni ausiliarie per la gestione dei pcb*/
void initHead(memaddr *pointer);
void setPointers(pcb_t *processPtr,pcb_t *setNext, pcb_t *setPrev);
void setValues(pcb_t *pointer);
void  initTail(memaddr *tailAddress, pcb_t *p);
int isPresent(pcb_t *tail, pcb_t *p);
void setPointerTree(pcb_t *p, pcb_t *prnt, pcb_t *nextSib,pcb_t *prevSib);
pcb_t* getLastChild(pcb_t *prnt);
#endif
