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

//Funciones del discordiador
void leer_consola();
void planificar_tripulantes();
void atender_ram();
void atender_imongo_store();
void *labor_tripulante_new(void * trip);

//Funciones de Planificacion
void planificar_cola_new();
void planificar_cola_ready();

//
void mostrar_lista_tripulantes();
void mostrar_tripulantes_new();


//-------------------------------
// Creacion de HILOS
void iniciarHiloConsola();
void planificar();
void atenderLaRam();
void atenderIMongoStore();
void crearHiloTripulante(t_tripulante * tripulante);

void hilo_cola_ready();
void hilo_cola_new();

#endif /* HILOSDISCORDIADOR_H_ */
