/*
 * sabotajes.c
 *
 *  Created on: 13 jul. 2021
 *      Author: utnso
 */


#include "sabotaje.h"


void mostrar_bloques(t_list * lista_bloques){
	void mostrar_bloque(int* bloque){
		log_info(logger,"%d",*bloque);
	}
	list_iterate(lista_bloques, (void*) mostrar_bloque);
}

void igualar_bitmap_contra_bloques(t_list * bloques_ocupados){
	pthread_mutex_lock(&superblock.mutex_superbloque);
	int i;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<list_size(bloques_ocupados) && i<cantidadDePosiciones;i++){
		int indice_bloque_ocupado = *(int*)list_get(bloques_ocupados,i);
		if(!bitarray_test_bit(superblock.bitmap,indice_bloque_ocupado)){
			bitarray_set_bit(superblock.bitmap,indice_bloque_ocupado);
			log_info(logger,"el bit %d estaba distinto",indice_bloque_ocupado);
		}
	}
	pthread_mutex_unlock(&superblock.mutex_superbloque);
}

void bloques_file_bitacora(_archivo_bitacora * archivo,t_list * lista_bloques){
		pthread_mutex_lock(&(archivo->mutex_file));
		char *resto_path = string_from_format("bitacora/%s%s", archivo->clave,".ims");
		t_config * config = config_create(resto_path);
		char ** bloques_ocupados = config_get_array_value(config,"BLOCKS");
		char * cadena = string_new();
		cadena = array_to_string(bloques_ocupados);
		log_info(logger,"%s",cadena);
		for(int i = 0 ; i<longitud_array(bloques_ocupados); i++){
			int * valor = malloc(sizeof(int));
			*valor =atoi(bloques_ocupados[i]);
			list_add(lista_bloques,valor);
		}
		pthread_mutex_unlock(&(archivo->mutex_file));

}



void sabotaje_bitmap_superbloque(){
	t_list * lista_bloques = list_create();
//	bloques_ocupados_file(archivo_basura);
	log_info(logger,"FSCK!!!!");

	bloques_ocupados_file(archivo_oxigeno,lista_bloques);
	bloques_ocupados_file(archivo_comida,lista_bloques);
	bloques_ocupados_file(archivo_basura,lista_bloques);

	for(int i = 0 ; i<list_size(archivos_bitacora);i++){
		_archivo_bitacora * archivo = (_archivo_bitacora*)list_get(archivos_bitacora,i);
		bloques_file_bitacora(archivo,lista_bloques);
	}

	mostrar_bloques(lista_bloques);


	igualar_bitmap_contra_bloques(lista_bloques);


	log_info(logger,"FSCKfin!!!!");
}



void bloques_ocupados_file(_archivo * archivo,t_list * lista_bloques){
	pthread_mutex_lock(&(archivo->mutex_file));
	char ** bloques_ocupados = config_get_array_value(archivo->metadata,"BLOCKS");
	char * cadena = string_new();
	cadena = array_to_string(bloques_ocupados);
	log_info(logger,"%s",cadena);
	for(int i = 0 ; i<longitud_array(bloques_ocupados); i++){
		int * valor = malloc(sizeof(int));
		*valor =atoi(bloques_ocupados[i]);
		list_add(lista_bloques,valor);
	}
	pthread_mutex_unlock(&(archivo->mutex_file));

}


void fsck(){
	sabotaje_bitmap_superbloque();
}






//FUNCIONES PARA BORRAR




void adulterar_bitmap(int signal){
	bitarray_clean_bit(superblock.bitmap,13);

	bitarray_clean_bit(superblock.bitmap,14);

	bitarray_clean_bit(superblock.bitmap,15);

	bitarray_clean_bit(superblock.bitmap,30);

}
