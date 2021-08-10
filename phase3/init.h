#include <pandos_types.h>
#include <pandos_const.h>
#ifndef INITIAL_H_INCLUDED
#define INITIAL_H_INCLUDED

/*dichiarazione puntatori globali*/
pcb_t *readyQ;
pcb_t *currentProcess;

/*dicharazione delle variabili globali*/
volatile unsigned int processCount;
unsigned int blockedCount;

// Per accedere
// al semaforo corrispondente al i-esimo tipo di dispositivo (Disk, Flash, ecc.) nella sua j-esima
// istanza è necessario accedere alla cella devSem[i-3][j] dove 7 ≤ i ≤ 3 e 7 ≤ j ≤ 0.
int devSem[4][8];

// Poiché
// ogni dispositivo terminale necessita di due semafori, è stato deciso di allocare la matrice terSem
// di dimensione 2x8, dove la riga 0 rappresenta i semafori di trasmissione mentre la riga 1
// rappresenta i semafori per la ricezione per ogni istanza di terminale. L’accesso avviene con un
// meccanismo analogo a quello per i dispositivi non terminali
int terSem[2][8];


int pseudoClock;
unsigned int processStartTime;
devregarea_t *bus_devReg_Area;



#endif