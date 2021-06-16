/*
 * estructuras.h
 *
 *  Created on: 6 jun. 2021
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_


typedef struct {
	char * nombre_tarea;
	uint32_t posicion_x;
	uint32_t posicion_y;
	uint32_t duracion;
}tareas;

typedef struct
{
	uint32_t identificador_pcb;
	tareas * tareas;
} pcb3;

typedef struct
{
	uint32_t identificador_tripulante;
	int socket_tcb; //VER
	char estado;
	uint32_t posicion_x;
	uint32_t posicion_y;
	uint32_t proxima_instruccion;
	pcb3 *puntero_pcb;
} tcb;

typedef struct
{
	uint32_t identificador_tripulante;
	char * id_posicion;
	char * tareas;
	int socket_tcb; //VER
	char estado;
	uint32_t proxima_instruccion;
	int patotaid;
	uint32_t cantidad_tripulantes;
} pcb;


typedef struct _infoHilos{
	int socket;
	pthread_t hiloAtendedor;
} infoHilos;





#endif /* ESTRUCTURAS_H_ */
