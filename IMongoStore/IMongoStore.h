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


typedef struct _tablaDeEntrada{
	char* clave;
	t_list * blocks;
	FILE * file;
} _archivo;

t_bitarray * bitArrayStorage;



t_bitarray * bitArrayStorage;


int server_fd;
char * archivo_oxigeno = "oxigeno.ims";
char * archivo_comida = "comida.ims";
char * archivo_basura = "basura.ims";

void manejadorDeHilos();
void inicializarMutex();
int agregar_en_bloque(char * cadena_caracteres,int indice);
uint32_t escribirEnMemoria(char* valor);
t_bitarray * crearBitArray(uint32_t cantBloques);
int calcularEntradasLibres();


void iniciar_super_block();

#endif /* IMONGOSTORE_H_ */
