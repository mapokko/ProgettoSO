#ifndef ASL_H_INCLUDED
#define ALS_H_INCLUDED
#include <pandos_types.h>
#include <pandos_const.h>

    /*creiamo 4 puntatori, alla testa e alla coda della lista dei semafori liberi, e alla testa e alla coda della lista dei semafori attivi*/
    extern semd_t *semdFree_h, *semdFree_tail;
    extern semd_t *semd_h, *semd_tail;

    /*funzioni di generazione e gestione dei semafori*/
    extern void initASL();
    extern int insertBlocked(int *semAdd, pcb_t *p);
    extern pcb_t* removeBlocked(int *semAdd);
    extern pcb_t* outBlocked(pcb_t *p);
    extern pcb_t* headBlocked(int *semAdd);

    /*funzione ausiliari per l'implementazioni delle funzioni sui semafori*/
    extern void freeSem(semd_t *sem);
    extern semd_t *isInserted(int *semAdd);
    extern semd_t *allocASL(int *semAdd);
    extern void insertSem(semd_t *newSem);
    extern void removeSem(semd_t *semDel);

#endif
