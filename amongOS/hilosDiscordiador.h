/*
 * hilosDiscordiador.h
 *
 *  Created on: 7 jun. 2021
 *      Author: utnso
 */




#ifndef HILOSDISCORDIADOR_H_
#define HILOSDISCORDIADOR_H_

#include "utils.h"
#include "tripulante.h"

void mostrar_tripulantes_new();
void leer_consola();


void planificar();
void planificar_tripulantes();

void planificar_cola_new();
void planificar_cola_ready();

void hilo_cola_ready();
void hilo_cola_new();


void atenderLaRam();
void iniciarHiloConsola();
void atenderIMongoStore();

#endif /* HILOSDISCORDIADOR_H_ */
