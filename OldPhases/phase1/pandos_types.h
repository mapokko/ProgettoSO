#ifndef PANDOS_TYPES_H_INCLUDED
#define PANDOS_TYPES_H_INCLUDED

/**************************************************************************** 
 *
 * This header file contains utility types definitions.
 * 
 ****************************************************************************/

#include <umps3/umps/types.h>
#include <pandos_const.h>



typedef unsigned int uint;
typedef signed int cpu_t;
typedef unsigned int memaddr;

/* process table entry type */
typedef struct pcb_t {
	/* process queue fields */
    struct pcb_t	*p_next;	/* ptr to next entry*/
    struct pcb_t	*p_prev; 	/* ptr to previous entry*/

	/* process tree fields */
    struct pcb_t	*p_prnt, 	/* ptr to parent*/
					*p_child,	/* ptr to 1st child*/
					*p_next_sib,	/* ptr to next sibling*/
					*p_prev_sib;	/* ptr to prev. sibling*/
	
	/* process status information */
    state_t			p_s;		/* processor state*/

	/* add more fields here */

	int *p_semAdd;

}  pcb_t, *pcb_PTR;

typedef struct semd_t {
	/* ptr to next element on queue */
	struct semd_t *s_next;

	/* ptr to the semaphore */
	int *s_semAdd;

	/* ptr to tail of the queue of procs.
	blocked on this sem. */
	pcb_PTR s_procQ;

} semd_t, *semd_PTR;
	
#endif
