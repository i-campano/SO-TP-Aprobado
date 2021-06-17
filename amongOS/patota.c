/*
 * patota.c
 *
 *  Created on: 11 jun. 2021
 *      Author: utnso
 */
#include "patota.h"



void crear_patota(){
	char * leido = readline("INGRESAR UBICACION TAREAS>");
	char * path = string_new();
	string_append(&path,leido);

	free(leido);

	char * tareasX = string_new();
    FILE *archivo = fopen(path, "r"); // Modo lectura
    char bufer[1000];         // Aquí vamos a ir almacenando cada línea
    int cantidad_tareas = 0;
    while (fgets(bufer, 1000, archivo))
    {
        // Aquí, justo ahora, tenemos ya la línea. Le vamos a remover el salto
        strtok(bufer, "\n");
        // La imprimimos, pero realmente podríamos hacer cualquier otra cosa
		string_append(&tareasX,bufer);
		string_append(&tareasX,"-");
		cantidad_tareas++;
    }
    char * tareasOk = string_substring_until(tareasX,string_length(tareasX)-1);



    log_info(logger,"%s",tareasOk);


//	char * leido = readline("INGRESAR TAREAS>");

//	free(leido);
	int longitud_tareas = string_length(tareasOk);

	leido = readline("CANTIDAD TRIPULANTES>");
	uint32_t cantidad_tripulantes = (uint32_t)atoi(leido);
	free(leido);

	leido = readline("ID PATOTA>");
	uint32_t patotaId = (uint32_t)atoi(leido);
	free(leido);


	char * posiciones = string_new();
	leido = readline("INGRESAR POSICIONES>");
	string_append(&posiciones,leido);

	free(leido);
	//string_append(&posiciones,"#12-3|4&#16-5|6");
	int longitud_posiciones = string_length(posiciones);

	char * claveGet = string_new();
	string_append(&claveGet,tareasOk);
	string_append(&claveGet,posiciones);

	int tamanioGet = 0;



	void* buffer_patota = crear_buffer_patota(longitud_tareas,
			longitud_posiciones, patotaId, cantidad_tripulantes,
			&tamanioGet, tareasOk, posiciones);

	sendRemasterizado(socketServerMiRam, CREAR_PATOTA,tamanioGet,buffer_patota);

	recvDeNotificacion(socketServerMiRam);
	log_info(logger,"PATOTA CREADA OK");

	for(int i = 0 ; i<cantidad_tripulantes; i++){
		tripulantes_creados++;
		int * id = malloc(sizeof(int));
		t_tripulante * _tripulante = (t_tripulante*)malloc(sizeof(t_tripulante));
		*id = tripulantes_creados;
		_tripulante->id = *id;
		_tripulante->patota_id = patotaId;
		_tripulante->cantidad_tareas = cantidad_tareas;
		log_info(logger,"Creando tripulante: %d de la patota id: %d",*id,patotaId);
		crearHiloTripulante(_tripulante);
	}


}



void* crear_buffer_patota(int longitud_tareas, int longitud_posiciones, uint32_t patotaId, uint32_t cantidad_tripulantes, int* tamanioGet, char* tareas, char* posiciones) {
	void* buffer_patota = malloc(longitud_tareas + longitud_posiciones + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(int) + sizeof(int));

	memcpy(buffer_patota + *tamanioGet, &longitud_tareas, sizeof(int));
	*tamanioGet += sizeof(int);

	memcpy(buffer_patota + *tamanioGet, tareas, longitud_tareas);
	*tamanioGet += longitud_tareas;

	memcpy(buffer_patota + *tamanioGet, &longitud_posiciones, sizeof(int));
	*tamanioGet += sizeof(int);

	memcpy(buffer_patota + *tamanioGet, posiciones, longitud_posiciones);
	*tamanioGet += longitud_posiciones;

	memcpy(buffer_patota + *tamanioGet, &patotaId, sizeof(uint32_t));
	*tamanioGet += sizeof(uint32_t);

	memcpy(buffer_patota + *tamanioGet, &cantidad_tripulantes,
			sizeof(uint32_t));
	*tamanioGet += sizeof(uint32_t);
	return buffer_patota;
}
