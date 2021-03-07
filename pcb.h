#include <pandos_types.h>
#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

void initPcbs();
void freePcb(pcb_t *p);
pcb_t *allocPcb();
pcb_t *mkEmptyProcQ();
int emptyProcQ(pcb_t *tp);
void insertProcQ(pcb_t **tp, pcb_t *p);
pcb_t *headProcQ(pcb_t **t);
pcb_t *removeProcQ(pcb_t **tp);
pcb_t *outProcQ(pcb_t **tp, pcb_t *p);
#endif
