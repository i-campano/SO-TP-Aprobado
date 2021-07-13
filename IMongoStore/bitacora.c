/*
 * bitacora.c
 *
 *  Created on: 11 jul. 2021
 *      Author: utnso
 */

#include "bitacora.h"

_archivo_bitacora * iniciar_archivo_bitacora(char * tripulante,char * key_file){

	_archivo_bitacora *archivo = (_archivo_bitacora*)malloc(sizeof(_archivo_bitacora));

	archivo->clave = string_new();
	string_append(&(archivo->clave),tripulante);

	char *resto_path = string_from_format("bitacora/%s%s", tripulante,".ims");

	if( access( resto_path, F_OK ) == 0 ) {
		log_info(logger,"YA EXISTE ARCHIVO BITACORA para: %s", tripulante);
		//Si tengo acceso al archivo, creo el config
		archivo->metadata = config_create(resto_path);
	} else {
		log_info(logger,"CREO NUEVO ARCHIVO BITACORA para: %s", tripulante);
		FILE * metadata = fopen(resto_path,"a+");
		close(metadata);
		archivo->metadata = config_create(resto_path);

		pthread_mutex_lock(&mutex_archivos_bitacora);
		list_add(archivos_bitacora,archivo);
		pthread_mutex_unlock(&mutex_archivos_bitacora);
	}

	FILE * metadata = fopen(resto_path,"a+");
//	FILE * metadata = open(resto_path,  O_APPEND | O_CREAT, 0644);
	int c = fgetc(metadata);
	if (c == EOF) {

		log_info(logger,"ESTA VACIO EL ARCHIVO BITACORA DE: %s", tripulante);
		config_set_value(archivo->metadata,"SIZE","0");
		config_set_value(archivo->metadata,"BLOCKS","[]");
		config_save_in_file(archivo->metadata,resto_path);


	} else {
	    ungetc(c, metadata);
	}





	pthread_mutex_init(&((archivo)->mutex_file), NULL);


	return archivo;

}



uint32_t write_archivo_bitacora(char* cadenaAGuardar,_archivo_bitacora * archivo){
	pthread_mutex_lock(&(archivo->mutex_file));

	uint32_t resultado;
	int bytesArchivo = config_get_int_value(archivo->metadata,"SIZE");

	log_debug(logger,"bytes archivo %s : %d",archivo->clave,bytesArchivo);

	pthread_mutex_lock(&_blocks.mutex_blocks);
	pthread_mutex_lock(&superblock.mutex_superbloque);
	//Chequeo si hay lugar en el ultimo bloque
	if(bytesArchivo%superblock.tamanio_bloque==0){

		//Si el ultiimo bloque esta completo -> creo los bloques nuevos que necesito para almacenar la cadena
		llenar_nuevo_bloque_bitacora(cadenaAGuardar, archivo);

	}else{
		//bytesArchivo : restas sucesivas para quedarme con el espacio OCUPADO del ultimo bloque.
		while(bytesArchivo>superblock.tamanio_bloque) bytesArchivo-=superblock.tamanio_bloque;

		int espacioLibreUltimoBloque = superblock.tamanio_bloque-bytesArchivo;

		char ** blocks = config_get_array_value(archivo->metadata,"BLOCKS");

		int longitud_blocks = longitud_array(blocks);
		char * last_block = blocks[longitud_blocks-1];

		if(string_length(cadenaAGuardar)<=espacioLibreUltimoBloque){

			log_info(logger,"indice de BLOQUE :%d con espacio para archivo: %s, ",atoi(last_block),archivo->clave);

			write_blocks_with_offset_bitacora(cadenaAGuardar,atoi(last_block),bytesArchivo);

			//TODO: podria armar un actualizar metadata mas generico con varios if
			actualizar_metadata_sin_crear_bloque_bitacora(archivo,cadenaAGuardar);

		}else{

			char * rellenoDeUltimoBloque  = string_substring_until(cadenaAGuardar,espacioLibreUltimoBloque);

			write_blocks_with_offset_bitacora(rellenoDeUltimoBloque,atoi(last_block),bytesArchivo);

			actualizar_metadata_sin_crear_bloque_bitacora(archivo,rellenoDeUltimoBloque);

			char * contenidoProximoBloque = string_substring_from(cadenaAGuardar,espacioLibreUltimoBloque);

			llenar_nuevo_bloque_bitacora(contenidoProximoBloque, archivo);

		}
	}
	log_info(logger,"%s: ----------- COPIA blocks.ims:",_blocks.fs_bloques);
	pthread_mutex_unlock(&superblock.mutex_superbloque);
	pthread_mutex_unlock(&_blocks.mutex_blocks);
	pthread_mutex_unlock(&(archivo->mutex_file));
	return 1;
}




void actualizar_metadata_sin_crear_bloque_bitacora(_archivo_bitacora * archivo,char * valorAux){

	int bytes = string_length(valorAux);
	int size = config_get_int_value(archivo->metadata,"SIZE");
	size+=bytes;

	config_set_value(archivo->metadata,"SIZE",string_itoa(size));

	config_save(archivo->metadata);

}


void actualizar_metadata_bitacora(_archivo_bitacora * archivo,int indice_bloque,char * valorAux){


	string_length(valorAux);
	int bytes = string_length(valorAux);

	int size = config_get_int_value(archivo->metadata,"SIZE");
	size+=bytes;
	config_set_value(archivo->metadata,"SIZE",string_itoa(size));


	char ** array;
	array = config_get_array_value(archivo->metadata,"BLOCKS");
	char ** nuevo  = agregar_en_array(array,string_itoa(indice_bloque));

	char * cadena = array_to_string(nuevo);
	int i = 0;
	while(nuevo[i]!=NULL){
		free(nuevo[i]);
		i++;
	}
	free(nuevo[i]);
	free(nuevo);
	config_set_value(archivo->metadata,"BLOCKS",cadena);
	config_save(archivo->metadata);
}



void llenar_nuevo_bloque_bitacora(char* cadenaAGuardar, _archivo_bitacora* archivo) {
	//Si el ultiimo bloque esta completo -> creo los bloques nuevos que necesito para almacenar la cadena
	int posicionesStorageAOcupar = calcular_cantidad_bloques_requeridos(
			cadenaAGuardar);
	int offsetBytesAlmacenados = 0;
	int bloqueslibres = calcular_bloques_libres();
	for (int i = 0; i < posicionesStorageAOcupar; i++) {
		char* valorAux = string_substring(cadenaAGuardar,
				offsetBytesAlmacenados, superblock.tamanio_bloque);
		int indice_bloque = obtener_indice_para_guardar_en_bloque_bitacora(valorAux);
		log_info(logger, "indice de bloque asignado a %s, :%d", archivo->clave,
				indice_bloque);
		write_blocks(valorAux, indice_bloque);
		actualizar_metadata_bitacora(archivo, indice_bloque, valorAux);
		bitarray_set_bit_monitor(indice_bloque);
		offsetBytesAlmacenados += superblock.tamanio_bloque;
	}
}

int obtener_indice_para_guardar_en_bloque_bitacora(char * valor){
	int lugares = calcular_cantidad_bloques_requeridos(valor);
	int cont = 0;
	int i;
	int resultado = 0;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(bitarray_test_bit(superblock.bitmap,i)){
			cont = 0;
		} else{
			cont++;
		}
		if(cont >= lugares){
			return i - lugares + 1;
			break;
		}
	}
	return 99999;
}

int calcular_cantidad_bloques_requeridos_bitacora(char* cadenaAGuardar){
	int cantidadBloques = string_length(cadenaAGuardar)/superblock.tamanio_bloque;

	if(string_length(cadenaAGuardar) % superblock.tamanio_bloque > 0 ){
		cantidadBloques++;
	}

	return cantidadBloques;
}


int write_blocks_with_offset_bitacora(char * cadena_caracteres,int indice,int offset) {
	t_bloque bloque;
	bzero(&bloque ,sizeof(t_bloque));

	int padding = sizeof(bloque.data) - offset-strlen(cadena_caracteres);
	char * pad = string_repeat('#',padding);
	char * cadena  = string_duplicate(cadena_caracteres);
	string_append(&cadena,pad);

	strcpy(bloque.data, cadena);

//	TODO : meter la validacion de bitarray aca  ¿
	memcpy(_blocks.fs_bloques + (indice*sizeof(t_bloque))+offset, cadena, string_length(cadena));
	return 1;
}



