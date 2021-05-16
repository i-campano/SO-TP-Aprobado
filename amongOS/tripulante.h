#ifndef _TRIPULANTE_H
#define _TRIPULANTE_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

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
	char estado;
	short int trabajando;
	short int fin_tareas;
	char* tarea; //Calculo que es necesario 50 50 SEGURIDAD
	pthread_t hilo_asociado;
}tripulante_t;

typedef struct {
	tripulante_t* dato;
	tripulante_t* sig;
}nodo_tripulante_t;

typedef struct {

	unsigned int grado_multitareas;
	unsigned int trip_en_exec;
	short int modo_planificacion; //FIFO = 0 RR = 1

} config_planif;
#endif
