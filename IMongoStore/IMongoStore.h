/*
 */

#ifndef IMONGOSTORE_H_
#define IMONGOSTORE_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include "utils.h"
#include "socket.h"
#include <sys/mman.h>

// CODIGO DE TAREAS
#define GENERAR_OXIGENO 90
#define GENERAR_COMIDA 91
#define GENERAR_BASURA 92
#define CONSUMIR_OXIGENO 110
#define CONSUMIR_COMIDA 111
#define DESCARTAR_BASURA 112


typedef struct _archivo{
	char* clave;
	t_list * blocks;
	FILE * file;
	char * contenido;
	pthread_mutex_t mutex_file;
} _archivo;

_archivo archivo_oxigeno;
_archivo archivo_comida;
_archivo archivo_basura;



t_bitarray * bitArrayStorage;



t_bitarray * bitArrayStorage;


int server_fd;


void manejadorDeHilos();
void inicializarMutex();
int agregar_en_bloque(char * cadena_caracteres,int indice);
uint32_t escribir_en_fs(char* valor);
t_bitarray * crearBitArray(uint32_t cantBloques);
int calcularEntradasLibres();
uint32_t leer_de_archivo(char* valor);
int leer_archivo(char * cadena_caracteres,int indice, _archivo archivo);
void escribir_en_archivo(_archivo archivo,int n_block,char * valorAux);


void iniciar_super_block();

#endif /* IMONGOSTORE_H_ */
