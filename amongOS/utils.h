/*
 * conexiones.h
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include "mylibrary.h"
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include <pthread.h>

t_log* logger;
int socketServerMiRam;	// PARA USUARIO
int socketListener,socketMaximo;	 // PARA SERVER
fd_set socketClientes, socketClientesAuxiliares; // PARA SERVER

t_list * hilosParaConexiones;


t_queue* planificacion_cola_new;
t_queue* planificacion_cola_ready;
t_queue* planificacion_cola_exec;
t_queue* planificacion_cola_bloq;
t_queue* planificcion_cola_fin;


//HILOS
pthread_t hiloConsola;
pthread_t hiloCoordinador;
pthread_t hiloPlanificador;


//HILOS PARA COLAS
pthread_t hiloColaReady;

//Semaforos
pthread_mutex_t mutexHilos;

typedef struct _infoHilos{
	int socket;
	pthread_t hiloAtendedor;
} infoHilos;

typedef struct {
	int id;
	char estado;
	short int trabajando;
	short int fin_tareas;
	char* tarea; //Calculo que es necesario 50 50 SEGURIDAD
	pthread_t hilo_asociado;
}t_nodo_tripulante;




int socketServerMiRam;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

#endif /* UTILS_H_ */
