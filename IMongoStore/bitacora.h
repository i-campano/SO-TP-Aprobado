/*
 * bitacora.h
 *
 *  Created on: 11 jul. 2021
 *      Author: utnso
 */

#ifndef BITACORA_H_
#define BITACORA_H_

#include "core.h"

typedef struct _archivo_bitacora{
	char* clave;
	t_list * blocks;
	FILE * file;
	t_config * metadata;
	pthread_mutex_t mutex_file;
} _archivo_bitacora;


uint32_t write_archivo_bitacora(char* cadenaAGuardar,_archivo_bitacora * archivo);


void actualizar_metadata_sin_crear_bloque_bitacora(_archivo_bitacora * archivo,char * valorAux);

void llenar_nuevo_bloque_bitacora(char* cadenaAGuardar, _archivo_bitacora* archivo);

int obtener_indice_para_guardar_en_bloque_bitacora(char * valor);

int calcular_cantidad_bloques_requeridos_bitacora(char* cadenaAGuardar);

int write_blocks_with_offset_bitacora(char * cadena_caracteres,int indice,int offset);
#endif /* BITACORA_H_ */
