#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

typedef struct
{
	uint32_t identificador_pcb;
} pcb;

typedef struct
{
	uint32_t identificador_tripulante;
	char estado;
	uint32_t posicion_x;
	uint32_t posicion_y;
	uint32_t proxima_instruccion;
	pcb *puntero_pcb;
} tcb;
