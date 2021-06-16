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
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/txt.h>
#include<commons/string.h>
#include<string.h>
#include "socket.h"

//VARIABLES DEL ARCHIVO DE CONFIGURACION
char* conf_PUNTO_MONTAJE;
uint32_t conf_PUERTO_IMONGO;
uint32_t conf_TIEMPO_SICRONIZACION;
char* conf_POSICIONES_SABOTAJE;
uint32_t conf_PUERTO_DISCORDIADOR;
char* conf_IP_DISCORDIADOR;
char* conf_ARCHIVO_OXIGENO_NOMBRE;
char* conf_ARCHIVO_COMIDA_NOMBRE;
char* conf_ARCHIVO_BASURA_NOMBRE;

//MUTEX PARA ARCHIVOS
pthread_mutex_t mut_ARCHIVO_OXIGENO;
pthread_mutex_t mut_ARCHIVO_COMIDA;
pthread_mutex_t mut_ARCHIVO_BASURA;


int server_fd;

typedef enum
{
	MENSAJE = 7,
	PAQUETE = 8
}op_code;

t_log* logger;

void *atenderNotificacion(void * paqueteSocket);

void iniciar_configuracion();

void iniciar_logger();

t_config* leer_config();

////////FUNCIONES DE TAREAS/////////
void generarDatos(uint32_t cantidad, char caracter);
void consumirDatos(uint32_t cantidad, char caracter);
void consumirDatos2(uint32_t cantidad, char caracter);

void descartarBasura();
////////FUNCIONES DE TAREAS/////////

#endif /* CONEXIONES_H_ */
