#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include "initProc.h"
#include "vmSupport.h"
#include <pandos_types.h>
#include <pandos_const.h>

int ioSem [UPROCMAX][2];


initioSem () {                                             // inizializzazione dei semafori dei device

  for (i = 0; i < UPROCMAX; i++) {

    ioSem[i][0] = 1;
    ioSem[i][1] = 1;

  }

}


initPageTable (ptEntry_t* pointer, int processno) {

  for (i = 0; i < MAXPAGES-1, i++) {

    pointer[i]->pt_entryHI = (processno << ASIDSHIFT) | ((0x80000+1) << VPNSHIFT);
    pointer[i]->pt_entryLO = DIRTYON | VALIDON;

  }

    pointer[i]->pt_entryHI = (0xBFFFF) | ((0x80000+1) << VPNSHIFT);
    pointer[i]->pt_entryLO = DIRTYON | VALIDON;

}


initUproc() {

  state_t prestate[UPROMAX];
  support_t presupport[UPROCMAX];

  for (i = 0; i < UPROCMAX; i++) {

    presupport[i].sup_asid = i + 1;
    presupport[i].sup_exceptContext[PGFAULTEXCEPT].c_pc = Pager;
    presupport[i].sup_exceptContext[GENERALEXCEPT].c_pc = supportExceptionshandler;
    presupport[i].sup_exceptContext[PGFAULTEXCEPT].c_status = TEBITON | IMON | KU;
    presupport[i].sup_exceptContext[GENERALEXCEPT].c_status = TEBITON | IMON | KU;
    presupport[i].sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = &(presupport[i].sup_stackTLB[499]);
    presupport[i].sup_exceptContext[GENERALEXCEPT].c_stackPtr = &(presupport[i].sup_stackGen[499]);
    ptEntry_t* pointer = presupport[i].sup_privatePgTbl;
    initPageTable(pointer, i+1);

    prestate[i].s_pc = 0x8000.00B0;
    prestate[i].s_t9 = 0x8000.00B0;
    prestate[i].s_sp = 0xC000.0000;
    prestate[i].status = TEBITON | IMON | KU;
    prestate[i].s_entryHI = i + 1;

    SYSCALL(CREATEPROCESS, prestate[i], presupport[i], 0);

  }

}

InstantiatorProcess () {                                   // inizializzazione dei processi

  initSwapStructs ();                                      // inizializzo la swap pool ed il suo semaforo

  initUproc();                                           // inizializzazione dei processi


}
