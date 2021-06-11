/*
 * patota.h
 *
 *  Created on: 11 jun. 2021
 *      Author: utnso
 */

#ifndef PATOTA_H_
#define PATOTA_H_

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
#include <semaphore.h>
#include "utils.h"


void crear_patota();

void* crear_buffer_patota(int longitud_tareas, int longitud_posiciones, uint32_t patotaId, uint32_t cantidad_tripulantes, int* tamanioGet, char* tareas, char* posiciones);

#endif /* PATOTA_H_ */
