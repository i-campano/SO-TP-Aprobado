/*
 * patota.c
 *
 *  Created on: 11 jun. 2021
 *      Author: utnso
 */
#include "patota.h"


//bool
void crear_patota(char * comando){
	socketServerMiRam = reConectarAServer(IP_MIRAM, PUERTO_MIRAM);
	if(socketServerMiRam<0){
		log_info(logger,"no me pude conectar con mi ram");
		return ;
	}else{

	t_list * list_trip_aux = list_create();

	char ** parametros = string_n_split(comando,5," ");

	char * tareasX = string_new();
    FILE *archivo = fopen(parametros[1], "r"); // Modo lectura
    char bufer[1000];         // Aquí vamos a ir almacenando cada línea
    int cantidad_tareas = 0;
    while (fgets(bufer, 1000, archivo))
    {
		string_append(&tareasX,bufer);
		cantidad_tareas++;
    }
    char * tareasOk = string_substring_until(tareasX,string_length(tareasX));



    log_info(logger,"%s",tareasOk);


	int longitud_tareas = string_length(tareasOk);

	uint32_t cantidad_tripulantes = (uint32_t)atoi(parametros[2]);

	uint32_t patotaId = (uint32_t)atoi(parametros[3]);

	char * posiciones = string_new();

	string_append(&posiciones,parametros[4]);

	int longitud_posiciones = string_length(posiciones);

	char * claveGet = string_new();
	string_append(&claveGet,tareasOk);
	string_append(&claveGet,posiciones);

	int tamanioGet = 0;

	void* buffer_patota = crear_buffer_patota(longitud_tareas,
			longitud_posiciones, patotaId, cantidad_tripulantes,
			&tamanioGet, tareasOk, posiciones);

	sendRemasterizado(socketServerMiRam, CREAR_PATOTA,tamanioGet,buffer_patota);
	/*if(recvDeNotificacion(socketServerMiRam) == PATOTA_CREADA){
		log_error(logger,"No se pudo crear");
		return false;
	}
	*/
	int state = recvDeNotificacion(socketServerMiRam);
	if (state == PATOTA_CREADA) {
	for(int i = 0 ; i<cantidad_tripulantes; i++){
		tripulantes_creados++;
		sendDeNotificacion(socketServerMiRam,tripulantes_creados);

		int * id = malloc(sizeof(int));
		t_tripulante * _tripulante = (t_tripulante*)malloc(sizeof(t_tripulante));
		*id = tripulantes_creados;
		_tripulante->id = tripulantes_creados;
		_tripulante->patota_id = patotaId;
		_tripulante->direccionLogica = recvDeNotificacion(socketServerMiRam);
		recvDeNotificacion(socketServerMiRam);
		sem_init(&_tripulante->creacion,0,0);

		list_add(list_trip_aux,_tripulante);

		log_info(logger,"Creando tripulante: %d de la patota id: %d ",*id,patotaId);
		crearHiloTripulante(_tripulante);
		free(id); //malloc linea 79 dentro de este while
	}
	}
	if(recvDeNotificacion(socketServerMiRam) == PATOTA_CREADA){
	while(list_size(list_trip_aux)!=0){
		t_tripulante * trip = list_remove(list_trip_aux,0);
		sem_post(&trip->creacion);
	}
	list_destroy(list_trip_aux);
	}
	else {
		log_error(logger,"No se pudo crear la patota");
	}
	//crear lista de ids y enviar
	//cuando el tripulante se conecte a miram, atenderlo, buscar su tcb y agregarlo a la lista de sockets conocidos
	free(buffer_patota); //Se hace el malloc dentro de crear_buffer_patota
	free(claveGet); //malloc linea 60
	free(posiciones); //stringNew linea 52
	free(tareasX);//stringNew linea 18
	free(tareasOk); // linea 31
	}
}
//Mallocs Revisados
void asignar_posicion(char** destino,char* posiciones,uint32_t creados) {
	char** posiciones_separadas = string_split(posiciones," ");
	uint32_t cantidad_posiciones = 0;
	while(posiciones_separadas[cantidad_posiciones] != NULL) {
		cantidad_posiciones++;
	}
	if(cantidad_posiciones <= creados){
		string_append(destino,"0|0");
	}
	else {
		string_append(destino,posiciones_separadas[creados]);
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
//Mallocs Revisados