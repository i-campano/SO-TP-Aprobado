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
#include"estructuras.h"

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


t_log* logger;
void mostrar_lista_patota();
void crear_pcb();
void *atenderNotificacion(void * paqueteSocket);


#endif /* CONEXIONES_H_ */
