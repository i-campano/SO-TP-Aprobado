/*
 * conexiones.h
 *
 *  Created on: 3 mar. 2019
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<pthread.h>
#include "estructuras.h"

#define IP "127.0.0.1"
#define PUERTO "5002"

int server_fd;

typedef enum
{
	MENSAJE = 7,
	PAQUETE = 8
}op_code;

t_list * lista_tcb;

t_list * lista_pcb;

t_list * lista_tripulantes;

pthread_mutex_t pthread_mutex_tcb_list;

pthread_mutex_t pthread_mutex_pcb_list;

typedef struct {
	int id;
	int patota_id;
	int cantidad_tareas;
	char* tarea; //Calculo que es necesario 50 50 SEGURIDAD
	pthread_t hilo_asociado;
	int socket;
	int instrucciones_ejecutadas;
	pthread_mutex_t ejecutadas;
	int fin;
	char estado;
	uint32_t ubi_x;
	uint32_t ubi_y;
}t_tripulante;


t_log* logger;
char * obtener_tarea(t_tripulante * tripulante);
void mostrar_patota(pcb* pcb);
void mostrar_lista_patota();
void crear_pcb();
void *atenderNotificacion(void * paqueteSocket);
void iniciarEstructurasAdministrativas();
void manejadorDeHilos();
#endif /* CONEXIONES_H_ */
