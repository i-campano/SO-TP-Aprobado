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
void planificar_cola_ready();
void planificar_cola_exec();
void planificar_cola_bloq();
void replanificar();


//
void mostrar_lista_tripulantes_new();
void mostrar_lista_tripulantes_fin();
void mostrar_lista_tripulantes_ready();
void mostrar_lista_tripulantes_bloq();
void mostrar_lista_tripulantes_exec();

void listar_tripulantes();



//-------------------------------
// Creacion de HILOS
void iniciarHiloConsola();
void planificar();
void atenderLaRam();
void atenderIMongoStore();
void crearHiloTripulante(t_tripulante * tripulante);

void hilo_cola_ready();
void hilo_cola_replanificar();
void hilo_cola_exec();
void hilo_cola_bloq();

void hilo_mostrar_tripulantes();


#endif /* HILOSDISCORDIADOR_H_ */
