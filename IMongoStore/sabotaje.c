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

	}
	pthread_mutex_unlock(&superblock.mutex_superbloque);

	log_trace(logger,"igualar_bitmap_contra_bloques: luego de limpiar");
	calcular_bloques_libres();

	pthread_mutex_lock(&superblock.mutex_superbloque);

	pthread_mutex_lock(&mutex_archivos_bitacora);

	for(int i=0; i<list_size(bloques_ocupados);i++){
		int* bloque_ocupado = (int*)list_get(bloques_ocupados,i);
		if(!bitarray_test_bit(superblock.bitmap,*bloque_ocupado)){
			bitarray_set_bit(superblock.bitmap,*bloque_ocupado);
			log_trace(logger,"igualar_bitmap_contra_bloques: el bit %d estaba distinto",*bloque_ocupado);
		}
	}

	pthread_mutex_unlock(&mutex_archivos_bitacora);
	pthread_mutex_unlock(&superblock.mutex_superbloque);
	log_trace(logger,"igualar_bitmap_contra_bloques: luego de igualar");
	calcular_bloques_libres();
}

void bloques_file_bitacora(_archivo_bitacora * archivo,t_list * lista_bloques){
		pthread_mutex_lock(&(archivo->mutex_file));
		char *resto_path = string_from_format("bitacora/%s%s", archivo->clave,".ims");
		t_config * config = config_create(resto_path);
		char ** bloques_ocupados = config_get_array_value(config,"BLOCKS");
		char * cadena = string_new();
		cadena = array_to_string(bloques_ocupados);
		log_trace(logger,"Bitacora: %s Bloques: %s",archivo->clave,cadena);
		for(int i = 0 ; i<longitud_array(bloques_ocupados); i++){
			int * valor = malloc(sizeof(int));
			*valor =atoi(bloques_ocupados[i]);
			list_add(lista_bloques,valor);
		}
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
	}
	pthread_mutex_unlock(&(archivo->mutex_file));

}


void fsck(){
	log_info(logger,"Ejecutando FSCK -> INICIO");
	sabotaje_bitmap_superbloque();
	log_info(logger,"Ejecutando FSCK -> FIN");
}






//FUNCIONES PARA BORRAR




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
