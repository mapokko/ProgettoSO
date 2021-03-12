#ifndef ASL_H_INCLUDED
#define ALS_H_INCLUDED
#include <pandos_types.h>
#include <pandos_const.h>

    /*funzioni di generazione e gestione dei semafori*/
    extern void initASL();
    extern int insertBlocked(int *semAdd, pcb_t *p);
    extern pcb_t* removeBlocked(int *semAdd);
    extern pcb_t* outBlocked(pcb_t *p);
    extern pcb_t* headBlocked(int *semAdd);

    /*funzione ausiliari per l'implementazioni delle funzioni sui semafori*/
    void freeSem(semd_t *sem);
    semd_t *isInserted(int *semAdd);
    semd_t *allocASL(int *semAdd);
    void *insertSem(semd_t *newSem);
    void removeSem(semd_t *semDel);

#endif
