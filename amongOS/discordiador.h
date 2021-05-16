#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_
#define RAM 0
#define FILE_SYSTEM 1

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include <pthread.h>

#include "utils.h"

typedef struct {
	int id;
	char estado;
	short int trabajando;
	short int fin_tareas;
	char* tarea; //Calculo que es necesario 50 50 SEGURIDAD
	pthread_t hilo_asociado;
}t_nodo_tripulante;

int realizar_operacion(char* mensaje,int conexion_mi_ram,int conexion_file_system,int conexion_tripulante);
t_log* iniciar_logger(void);
t_config* leer_config(void);
void leer_consola(t_log* logger,int conexion_ram,int conexion_fs,int conexion_trip);
t_paquete* armar_paquete();
int terminar_programa(t_log* logger,t_config* config,int conexion[2]);
static t_nodo_tripulante *nodo_tripulante_create(char estado, short int fin_tareas, short int trabajando, char *tarea);
static void nodo_tripulante_destroy(t_nodo_tripulante *self);
#endif /* DISCORDIADOR_H_ */
