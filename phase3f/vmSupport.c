#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include <pandos_types.h>
#include <pandos_const.h>
#include "vmsSupport.h"

 swap_t *sw_p;
 int semSP;
 int fifono;


 void initSwapStructs () {

   fifono=0;
   initSwapPool();
   initSwapPoolSem();

 }

 void initSwapPoolSem () {

   semSP = 1;

 }

 void initSwapPool () {

   sw_p = SWAPPOOL;

   for (i = 0; i < USERPGTBLSIZE; i++) {

     sw_p[i].sw_asid = -1;
     sw_p[i].sw_page = 0;
     sw_p[i].sw_pt = NULL;

   }

 }

 swap_t* pagingFIFO() {

   fifono = fifono + 1;
   return sw_p[fifono%USERPGTBLSIZE];

 }

 void updatePageTable (ptEntry_t* pointer) {

   pointer->pt_entryLO = pointer->pt_entryLO & !(VALIDON);

 }

 TLBupdate() {

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
  unsigned int cause = sp_p -> sup_exceptState[0].s_cause;

  if (((cause & GETEXECCODE) >> 2) == 1 ) {

    //questa eccezione viene trattata come una program trap (capitolo 4.8 pandos)

  }

  SYSCALL(PASSEREN, &semSP, 0, 0);
  unsigned int p = ((getENTRYHI() & GETPAGENO ) >> 12);
  sw_p* pointer = pagingFIFO();

  if ((pointer->sw_asid) != -1) {



    updatePageTable (pointer->sw_pt);
    TLBupdate();

  }







}
