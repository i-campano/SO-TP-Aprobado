/*
 * sabotajes.c
 *
 *  Created on: 13 jul. 2021
 *      Author: utnso
 */


#include "sabotaje.h"


void mostrar_bloques(t_list * lista_bloques){
	void mostrar_bloque(int* bloque){
		log_trace(logger,"mostrar_bloques: %d",*bloque);
	}
	list_iterate(lista_bloques, (void*) mostrar_bloque);
}

//TODO: no se usa por ahora. -> borrar
int buscar_bloque(t_list * bloques_ocupados,int i){
	bool por_indice(void* indice){
		int* bloque = (int*) indice;
		return bloque==i? true : false;
	}
	int* indice_bloque= list_find(bloques_ocupados, por_indice);
	return *indice_bloque;
}

void igualar_bitmap_contra_bloques(t_list * bloques_ocupados){
	log_trace(logger,"igualar_bitmap_contra_bloques: antes de igualar");
	calcular_bloques_libres();

	int cantidadDePosiciones = superblock.cantidad_bloques;
	pthread_mutex_lock(&superblock.mutex_superbloque);
	for(int i=0; i<cantidadDePosiciones;i++){
		bitarray_clean_bit(superblock.bitmap,i);

		//HAGO EL CLEAN DEL BITARRAY

	}
	pthread_mutex_unlock(&superblock.mutex_superbloque);

	log_trace(logger,"igualar_bitmap_contra_bloques: luego de limpiar");
	calcular_bloques_libres_ONLY();

	pthread_mutex_lock(&superblock.mutex_superbloque);

	pthread_mutex_lock(&mutex_archivos_bitacora);
	//TODO: ver si hacer esto mismo sin limpiar todos los bits al principio
	bool saboteado = false;
	for(int i=0; i<list_size(bloques_ocupados);i++){
		int* bloque_ocupado = (int*)list_get(bloques_ocupados,i);
		if(!bitarray_test_bit(superblock.bitmap,*bloque_ocupado)){
			bitarray_set_bit(superblock.bitmap,*bloque_ocupado);
			log_trace(logger,"igualar_bitmap_contra_bloques: el bit %d estaba distinto",*bloque_ocupado);
			saboteado = true;
		}
	}

	memcpy(superblock.bitmapstr + 2*sizeof(uint32_t), (superblock.bitmap->bitarray), (superblock.cantidad_bloques/8));
	msync(superblock.bitmapstr, 2*sizeof(uint32_t)+ (superblock.cantidad_bloques/8), MS_SYNC);

//	if(saboteado){
//		log_info(logger,"Habia diferencias entre el bitmap y la metadata");
//
//	}else{
//		log_info(logger,"TODO OK: No habia diferencias entre el bitmap y la metadata");
//	}

	pthread_mutex_unlock(&mutex_archivos_bitacora);
	pthread_mutex_unlock(&superblock.mutex_superbloque);
	log_trace(logger,"igualar_bitmap_contra_bloques: luego de igualar");
	calcular_bloques_libres_ONLY();
}

void bloques_file_bitacora(_archivo_bitacora * archivo,t_list * lista_bloques){
		pthread_mutex_lock(&(archivo->mutex_file));
		char *aux = NULL;
		char *path_files = NULL;
		aux = string_duplicate(conf_PUNTO_MONTAJE);
		path_files = string_duplicate(conf_PATH_BITACORA);
		string_append_with_format(&aux, "%s", path_files);
		char *resto_path = string_from_format("%s%s", archivo->clave,".ims");
		string_append_with_format(&aux, "%s", resto_path );
		log_trace(logger,"%s",aux);
		t_config * config = config_create(aux);
		char ** bloques_ocupados = config_get_array_value(config,"BLOCKS");
		char * cadena = string_new();
		cadena = array_to_string(bloques_ocupados);
		log_trace(logger,"Bitacora: %s Bloques: %s",archivo->clave,cadena);
		for(int i = 0 ; i<longitud_array(bloques_ocupados); i++){
			int * valor = malloc(sizeof(int));
			*valor =atoi(bloques_ocupados[i]);
			list_add(lista_bloques,valor);
			free(bloques_ocupados[i]);
		}
		free(cadena);
		free(resto_path);
		config_destroy(config);

		pthread_mutex_unlock(&(archivo->mutex_file));

}

void obtener_todos_los_bloques_desde_metedata(t_list* lista_bloques) {
	bloques_ocupados_file(archivo_oxigeno, lista_bloques);
	bloques_ocupados_file(archivo_comida, lista_bloques);
	bloques_ocupados_file(archivo_basura, lista_bloques);
	pthread_mutex_lock(&mutex_archivos_bitacora);
	for (int i = 0; i < list_size(archivos_bitacora); i++) {
		_archivo_bitacora* archivo = (_archivo_bitacora*) list_get(
				archivos_bitacora, i);
		bloques_file_bitacora(archivo, lista_bloques);
	}
	pthread_mutex_unlock(&mutex_archivos_bitacora);
}

void sabotaje_cantidad_bloques_superbloque(){

}


void sabotaje_bitmap_superbloque(){
	t_list * lista_bloques = list_create();

	log_info(logger,"FSCK: Chequeando BITMAP -> INICIO");

	obtener_todos_los_bloques_desde_metedata(lista_bloques);

//	mostrar_bloques(lista_bloques);

	igualar_bitmap_contra_bloques(lista_bloques);

	log_info(logger,"FSCK: Chequeando BITMAP -> FIN");
}




void bloques_ocupados_file(_archivo * archivo,t_list * lista_bloques){
	pthread_mutex_lock(&(archivo->mutex_file));
	char ** bloques_ocupados = config_get_array_value(archivo->metadata,"BLOCKS");
	char * cadena = string_new();
	cadena = array_to_string(bloques_ocupados);
	log_trace(logger,"Bloques ocupados por archivo: %s",cadena);
	for(int i = 0 ; i<longitud_array(bloques_ocupados); i++){
		int * valor = malloc(sizeof(int));
		*valor =atoi(bloques_ocupados[i]);
		list_add(lista_bloques,valor);
		free(bloques_ocupados[i]);

		//TODO :Free
	}
	free(cadena);

	pthread_mutex_unlock(&(archivo->mutex_file));

}

void corregir_block_count_file(_archivo * archivo,char * name_file){
	pthread_mutex_lock(&(archivo->mutex_file));
	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	path_files = string_duplicate(conf_PATH_FILES);
	string_append_with_format(&aux, "%s%s", path_files,name_file);
	log_trace(logger,"aux : %s",aux);
	archivo->metadata = config_create(aux);
	char ** bloques_ocupados = config_get_array_value(archivo->metadata,"BLOCKS");
	int cantidad_bloques = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
	if(cantidad_bloques!=longitud_array(bloques_ocupados)){
		log_info(logger,"Estaba saboteado el BLOCK_COUNT de %s",archivo->clave);
		config_set_value(archivo->metadata,"BLOCK_COUNT",string_itoa(longitud_array(bloques_ocupados)));
		config_save(archivo->metadata);
	}
	pthread_mutex_unlock(&(archivo->mutex_file));

}

void contrastar_cantidad_bloques(){
	struct stat info;
	bzero(&info, sizeof(struct stat));
	FILE *fptr;

	char *aux = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	string_append_with_format(&aux, "%s", "blocks.ims");
	log_info(logger,"aux : %s",aux);

	if (stat(aux, &info) != 0) {
		log_error(logger, "No se puede hacer stat en el bitmap file");
		exit_failure();
	}
	if (info.st_size > 0) {
		uint32_t cantidad_bloques_blocks = info.st_size / superblock.tamanio_bloque;
		log_info(logger,"Cantidad de blocks (desde stat) %d",cantidad_bloques_blocks);
		log_info(logger,"Cantidad de blocks (desde config) %d",superblock.cantidad_bloques);
		if(cantidad_bloques_blocks!=superblock.cantidad_bloques){
			log_info(logger,"La cantidad de bloques fue saboteada, pero ya fue reparada");
			superblock.cantidad_bloques = cantidad_bloques_blocks;
			memcpy(superblock.bitmapstr, &(superblock.cantidad_bloques), sizeof(uint32_t));

			msync(superblock.bitmapstr, (sizeof(uint32_t)), MS_SYNC);
		}
		else{
			log_info(logger,"La cantidad de bloques no fue saboteada");
		}
	}
}

void contrastar_size_vs_bloques_files(){
	corregir_block_count_file(archivo_comida,conf_ARCHIVO_COMIDA_NOMBRE);
	corregir_block_count_file(archivo_basura,conf_ARCHIVO_BASURA_NOMBRE);
	corregir_block_count_file(archivo_oxigeno,conf_ARCHIVO_OXIGENO_NOMBRE);
}

void contrastar_tamanio_archivos_de_recurso(){
	obtener_tamanio_archivo_de_recurso(archivo_comida,conf_ARCHIVO_COMIDA_NOMBRE);
	obtener_tamanio_archivo_de_recurso(archivo_oxigeno,conf_ARCHIVO_OXIGENO_NOMBRE);
	obtener_tamanio_archivo_de_recurso(archivo_basura,conf_ARCHIVO_BASURA_NOMBRE);
}



void fsck(){
	log_info(logger,"Ejecutando FSCK -> INICIO");


	sabotaje_bitmap_superbloque(); // Revisado -> posible mejora TODO
	contrastar_tamanio_archivos_de_recurso(); // Revisado
	contrastar_cantidad_bloques(); // Revisado -> comportamiento: Truncando el archivo por linea de comandos -> actualiza la cantidad de bloques dentro del archivo de superblocks
	contrastar_size_vs_bloques_files(); // Revisado


	log_info(logger,"Ejecutando FSCK -> FIN");
	sleep(conf_TIEMPO_SABOTAJE);
}






//FUNCIONES PARA BORRAR



void terminar_imongo(int signal){
	log_error(logger, "MURIENDO CON ELEGANCIA...");
//	liberarStorage();

	exit(-1);

}

void adulterar_bitmap(int signal){
	pthread_mutex_lock(&superblock.mutex_superbloque);

	for(int i = 0; i<superblock.cantidad_bloques; i++){
		bitarray_clean_bit(superblock.bitmap,i);
	}

	pthread_mutex_unlock(&superblock.mutex_superbloque);
}

void adulterar_bitmap2(int signal){
	pthread_mutex_lock(&superblock.mutex_superbloque);
	bitarray_clean_bit(superblock.bitmap,13);

	bitarray_clean_bit(superblock.bitmap,14);

	bitarray_clean_bit(superblock.bitmap,15);

	bitarray_clean_bit(superblock.bitmap,30);
	pthread_mutex_unlock(&superblock.mutex_superbloque);
}
