#ifndef PANDOS_TYPES_H_INCLUDED
#define PANDOS_TYPES_H_INCLUDED

/****************************************************************************
 *
 * This header file contains utility types definitions.
 *
 ****************************************************************************/

#include <umps3/umps/types.h>
#include "pandos_const.h"

typedef unsigned int uint;
typedef signed int   cpu_t;
typedef unsigned int memaddr;
typedef state_t *state_PTR;

typedef struct ptEntry_t {
    unsigned int pt_entryHI;
    unsigned int pt_entryLO;
} ptEntry_t;

typedef struct context_t {
    unsigned int c_stackPtr;
    unsigned int c_status;
    unsigned int c_pc;
} context_t;

typedef struct support_t {
    int       sup_asid;             /* process ID					*/
    state_t   sup_exceptState[2];   /* old state exceptions			*/
    context_t sup_exceptContext[2]; /* new contexts for passing up	*/
    pteEntry_t sup_privatePgTbl[USERPGTBLSIZE]; /* the page table*/
    int sup stackTLB[500];
    int sup stackGen[500];
} support_t;

/* process table entry type */
typedef struct pcb_t {
    /* process queue fields */
    struct pcb_t *p_next; /* ptr to next entry			*/
    struct pcb_t *p_prev; /* ptr to previous entry		*/

    /* process tree fields */
    struct pcb_t *p_prnt, /* ptr to parent				*/
        *p_child,         /* ptr to 1st child				*/
        *p_next_sib,      /* ptr to next sibling 			*/
        *p_prev_sib;      /* ptr to prev. sibling			*/

    /* process status information */
    state_t p_s;                /* processor state        */
    cpu_t   p_time;             /* cpu time used by proc		*/

    /* add more fields here */
    int *p_semAdd;
    support_t *p_supportStruct;

} pcb_t, *pcb_PTR;

typedef struct semd_t {
	/* ptr to next element on queue */
	struct semd_t *s_next;

	/* ptr to the semaphore */
	int *s_semAdd;

	/* ptr to tail of the queue of procs.
	blocked on this sem. */
	pcb_PTR s_procQ;

} semd_t, *semd_PTR;

typedef struct swap_t  {
    int         sw_asid;   /* ASID			*/
    int         sw_page;
    ptEntry_t *sw_pt;
} swap_t;

#endif
