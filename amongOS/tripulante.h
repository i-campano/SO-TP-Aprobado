#ifndef _TRIPULANTE_H
#define _TRIPULANTE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils.h"

int tripulantes_creados;

//BOOL
#define TRUE 1
#define FALSE 0

//Estados de los trip segun cola
#define NEW 'N'
#define READY 'R'
#define EXEC 'E'
#define BLOQ 'B'
#define BLOQ_SAB 'S' //QUIZAS ESTE NO SIRVA AHORA NO SE
#define FIN 'F'

//Estado Planificacion
#define ACTIVE 1
#define PAUSE 0

//Modos planificacion
#define FIFO 0
#define RR 1

typedef struct {
	int id;
	int patota_id;
	int cantidad_tareas;
	char* tarea; //Calculo que es necesario 50 50 SEGURIDAD
	pthread_t hilo_asociado;
	int socket;
	sem_t ready;
	sem_t new;
	sem_t exec;
	sem_t bloq;
	int instrucciones_ejecutadas;
	pthread_mutex_t ejecutadas;
	int fin;
	char estado;
}t_tripulante;


typedef struct {

	unsigned int grado_multitareas;
	unsigned int trip_en_exec;
	short int modo_planificacion; //FIFO = 0 RR = 1

} config_planif;


void *labor_tripulante_new(void * id_tripulante);

void enviar_tarea_a_ejecutar(int socketMongo, int id, char* claveNueva);


#endif
