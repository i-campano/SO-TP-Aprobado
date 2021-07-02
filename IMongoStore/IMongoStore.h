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


void manejadorDeHilos();
void inicializarMutex();
int write_blocks(char * cadena_caracteres,int indice);
uint32_t write_archivo(char* valor,_archivo archivo);
t_bitarray * crear_bit_array(uint32_t cantBloques);
int calcular_bloques_libres();
uint32_t leer_contenido_archivo(char* valor,_archivo * arch);
int leer_metadata_archivo(char * cadena_caracteres,int indice, _archivo archivo);
void actualizar_metadata(_archivo archivo,int n_block,char * valorAux);
void iniciar_archivo(char * name_file,_archivo *archivo,char * key_file);
void iniciar_blocks();
void consumir_arch(_archivo * archivo,int cantidadAConsumir);
void remover_bloque(int indice,_archivo * archivo);
int obtener_contenido_bloque(int indice,char ** bloqueReturned);
int obtener_bloque(int indice);
void actualizar_metadata_borrado(_archivo * archivo);

void iniciar_super_block();

#endif /* IMONGOSTORE_H_ */
