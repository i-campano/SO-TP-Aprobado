#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include "utils.h"
#include "socket.h"
#include "hilosDiscordiador.h"
#include "tripulante.h"

typedef struct patota{
	uint32_t patota_id;
    uint32_t cantidad_tripulantes;
    char * tareas;
    int longitud_tareas;
    int longitud_posiciones;
    char * posiciones;
    int socketMIRAM;
} patota;

t_config* config;


char* valor;

char* ip;

#endif /* DISCORDIADOR_H_ */
