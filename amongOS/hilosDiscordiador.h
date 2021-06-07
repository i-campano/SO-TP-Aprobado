/*
 * hilosDiscordiador.h
 *
 *  Created on: 7 jun. 2021
 *      Author: utnso
 */




#ifndef HILOSDISCORDIADOR_H_
#define HILOSDISCORDIADOR_H_



#include "utils.h"

pthread_t hiloColaReady;

void planificar_cola_new();
void planificar_cola_ready();



#endif /* HILOSDISCORDIADOR_H_ */
