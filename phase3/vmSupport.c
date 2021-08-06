#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include <pandos_types.h>
#include <pandos_const.h>
#include "vmSupport.h"

 swap_t *sw_p;
 int semSP;
 int fifono;


 void initSwapStructs () {                                 //inizializzazione strutture

   fifono=0;
   initSwapPool();
   initSwapPoolSem();

 }

 void initSwapPoolSem () {                                 //inizializzazione semaforo swap pool

   semSP = 1;

 }

 void initSwapPool () {                                    //inizializzazione della swap pool

   sw_p = SWAPPOOL;

   for (i = 0; i < USERPGTBLSIZE; i++) {

     sw_p[i].sw_asid = -1;
     sw_p[i].sw_page = 0;
     sw_p[i].sw_pt = NULL;

   }

 }

 swap_t* pagingFIFO() {                                    //algoritmo di paging FIFO

   fifono = fifono + 1;
   return sw_p[fifono%USERPGTBLSIZE];

 }

 void PageTableupdate (ptEntry_t* pointer) {               //aggiorno la page table di un processo

   pointer->pt_entryLO = pointer->pt_entryLO & !(VALIDON);

 }

 TLBupdate() {                                             //da rivedere

   TLBP();

   if (Index.P == 0) {

     TLBLCR();

   } else {

     TLBWI();

   }

 }


void Pager () {

  p_supportStruct *sp_p;
  SYSCALL(GETSUPPORTPTR, 0, 0, 0);
  sp_p = currentState -> reg_v0
  unsigned int cause = sp_p -> sup_exceptState[0].s_cause; // ottendo il registro di supporto dal processo attualmente incorso

  if (((cause & GETEXECCODE) >> 2) == 1 ) {                // controllo se deve avvenire una trap

                                                           // questa eccezione viene trattata come una program trap (capitolo 4.8 pandos)

  }

  SYSCALL(PASSEREN, &semSP, 0, 0);                         // faccio una p function sul semaforo della swap pool
  unsigned int p = ((getENTRYHI() & GETPAGENO ) >> 12);    // ottengo il numero della pagina
  sw_p* pointer = pagingFIFO();                            // aggiorno la swap table

  if ((pointer->sw_asid) != -1) {

    PageTableupdate (pointer->sw_pt);
    TLBupdate();
    writedata();                                            //da implementare

  }

  int pageno = fifono%USERPGTBLSIZE;                                           //aggiorno la swap pool con le informazioni del nuovo processo
  sw_p[pageno].sw_asid = currentProcess->p_supportStruct->sup_asid;
  sw_p[pageno].sw_page = p;
  sw_p[pageno].sw_pt = currentProcess->p_supportStruct->sup_privatePgTbl[p];

  currentProcess->p_supportStruct->sup_privatePgTbl[p]->pt_entryLO = currentProcess->p_supportStruct->sup_privatePgTbl[p]->pt_entryLO & VALIDON;

  TLBupdate();

  SYSCALL(VERHOGEN, &semSP, 0, 0);

  LDST();

}
