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
}t_tarea;


typedef struct
{
	uint32_t identificador_pcb;
	t_tarea * tareas;
} pcb3;

typedef struct
{
	uint32_t identificador_tripulante;
	int socket_tcb; //VER
	char estado;
	uint32_t posicion_x;
	uint32_t posicion_y;
	uint32_t proxima_instruccion;
} tcb;

typedef struct
{
	uint32_t identificador_tripulante;
	char * id_posicion;
	int socket_tcb; //VER
	char estado;
	uint32_t proxima_instruccion;
	int patotaid;
	uint32_t cantidad_tripulantes;
	t_list * tareas_list;
} pcb;




typedef struct _infoHilos{
	int socket;
	pthread_t hiloAtendedor;
} infoHilos;





#endif /* ESTRUCTURAS_H_ */
