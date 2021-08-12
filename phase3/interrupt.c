#include "exceptions.h"
#include "init.h"
#include "scheduler.h"
#include "syscall.h"
#include <pandos_types.h>
#include <pandos_const.h>

memaddr *getDevReg(int devNumber, int interruptingDeviceNumber);

/*gestore degli interrupt*/
void interruptHandler(){
    /*estraggo l'interrupt mask*/
	unsigned int interruptCode;
	interruptCode = currentState->cause & CLEAREXECCODE;

    /*individio il primo bit a 1*/
	if(interruptCode & LOCALTIMERINT){
		PLTHandler();
	}
	else if(interruptCode & TIMERINTERRUPT){
		timerHandler();
	}
	else if(interruptCode & DISKINTERRUPT){
        IODeviceHandler(DISKINT);
	}
	else if(interruptCode & FLASHINTERRUPT){
        IODeviceHandler(FLASHINT);
	}
	else if(interruptCode & (FLASHINTERRUPT << 1)){ /*interrupt del Network Device*/
        IODeviceHandler(NETWINT);
	}
	else if(interruptCode & PRINTINTERRUPT){
        IODeviceHandler(PRNTINT);
	}
	else if(interruptCode & TERMINTERRUPT){
		terminalHandler();
	}
}

/*gestore della linea di interrupt del PLT*/
void PLTHandler(){
    /*aggiorno p_time e
     *copio stato corrente
    */
	updateCPUtime();
	memcpy(currentState, &(currentProcess->p_s), sizeof(state_t));
	
    /*inserisco pcb nel readyQ
     *e faccio ACK del PLT con
     *valoer simboli e chiamo scheduler
    */
	insertProcQ(&readyQ, currentProcess);
	setTIMER(100000);
	scheduler();
}

/*gestore interrupt timer*/
void timerHandler(){
	/*V operation finché tutti
     *i pcb sono sbloccati
    */
	while(pseudoClock < 0){
		verhogen(&pseudoClock);
	}
	
    /*ricarico interrupt timer
     *e controllo se c'è un
     *processo corrente
    */
	LDIT(PSECOND);
	checkCurrent();

    /*se c'è, riprendi esecuzione*/
	LDST(currentState);
	
}

/*gestore interrupt di Disk, Flash, Network e Printer*/
void IODeviceHandler(int devNumber){
    /*dichiarazione variabili per eseguire la gestione*/
    unsigned int interruptingDeviceNum, status;
    dtpreg_t *deviceRegister;
    pcb_t *unblocked;
    
    /*estrazione del registro del device in interrupt*/
    interruptingDeviceNum = getLine(getBitmap(devNumber));
	deviceRegister = getDevReg(devNumber, interruptingDeviceNum);

    /*estrazione status dal device register*/
    status = deviceRegister->status;
    
    /*V operation sul semaforo del device*/
    unblocked = verhogen(&(devSem[devNumber - 3][interruptingDeviceNum]));
    /*se V operation OK, scrivo status nel registro reg_v0*/
    if(unblocked != NULL){
        unblocked->p_s.reg_v0 = status;
    }
    /*ACK del interrupt*/
    deviceRegister->command = 1;

    /*se c'è un processo in esecuzione, LDST()
     *altrimenti chiamo scheduler
    */
    checkCurrent();
    LDST(currentState);
}

/*gestore interrupt terminale*/
void terminalHandler(){
    /*dichiarazione variabili per eseguire la gestione*/
	unsigned int interruptingDeviceNum, status;
    termreg_t *terminalRegister;
	pcb_t *unblockedPcb = NULL;

    /*estrazione del registro del terminale in interrupt*/
	interruptingDeviceNum = getLine(getBitmap(TERMINT));
	terminalRegister = getDevReg(TERMINT, interruptingDeviceNum);

    /*estrazione status della
     *ricezione dal device register
    */
	status = terminalRegister->recv_status;
    
    /*2 cicli:
     *1: gestione interrupt da ricezione
     *2: gestione interrutp da trasmissione
    */
    for(int i = 0; i <= 1; i++){
        /*se device non è busy*/
        if((status & 0xff) != 3){
            /*diamo ACK*/
            if(i == 0){
                if((status & 0xff) == 5){
                    terminalRegister->recv_command = 1;
                } 
            }
            else{
                if((status & 0xff) == 5){
                    terminalRegister->transm_command = 1;
                } 
            }

            /*V operation sul semaforo*/
            if((status & 0xff) == 5){
                unblockedPcb = verhogen(&(terSem[i][interruptingDeviceNum]));
            }

            /*se V operation OK, scrivo status nel registro reg_v0*/
		    if(unblockedPcb != NULL){
		        unblockedPcb->p_s.reg_v0 = status;
            }
        }
        /*passo a status della trasmissione*/
        status = terminalRegister->transm_status;
        unblockedPcb = NULL;
    }
	
    /*se c'è un processo in esecuzione, LDST()
     *altrimenti chiamo scheduler
    */
	checkCurrent();
	LDST(currentState);

}

/*FUNZIONI AUSILIARIE*/

/*verifica se è presente un
 *processo in esecuzione
 *se non c'è, chiama scheduler
*/
void checkCurrent(){
	if(currentProcess == NULL){
		scheduler();
	}
}

/*prende il tipo di device e ritorna il bitmap
 *dei interrupt delle due 8 istanze
*/
int getBitmap(int devNumber){
	return bus_devReg_Area->interrupt_dev[devNumber - 3];
}

/*prende un bitmap, restituisce
 *l'indice del primo bit a 1*/
int getLine(int bitmap){
	unsigned int scanner, line;
	scanner = 1;
    /*mantiene l'indice del primo bit a 1*/
	line = 0;

    /*cicla finché non trova un 1*/
	while(!(bitmap & scanner)){
		scanner *= 2;
		line++;
	}

	return line;
}

/*calcola l'indirizzo dove comincia il device register*/
memaddr *getDevReg(int devNumber, int interruptingDeviceNumber){
	return DEVICEREGISTERBASE + ((devNumber - 3) * 0x80) + ((interruptingDeviceNumber) * 0x10);
}

FERMATIint(){}