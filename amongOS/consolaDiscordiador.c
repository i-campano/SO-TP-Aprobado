/*
 * consolaDiscordiador.c
 *
 *  Created on: 14 jun. 2021
 *      Author: utnso
 */

#include "consolaDiscordiador.h"

void leer_consola() {
	log_info(logger,"INGRESE UN COMANDO: ");
	tripulantes_creados = 0;
	char* leido = readline(">");
	while(strncmp(leido, "TERM", 4) != 0) {
		log_info(logger, leido);

		if(strncmp(leido, "DETENER", 1) == 0){
			log_info(logger,"PLANIFICACION DETENIDA !!!: ");
			sem_wait(&detenerReaunudarEjecucion);
		}
		else if(strncmp(leido, "ACTUALIZACIONES_MONGO", 1) == 0){
			log_info(logger,"actualizaciones mongo activado !!!: ");
			sem_post(&activar_actualizaciones_mongo);
		}
		else if(strncmp(leido, "MONGO_DETENER", 5) == 0){
			log_info(logger,"actualizaciones mongo detenido!!!: ");
			sem_wait(&activar_actualizaciones_mongo);
		}
		else if(strncmp(leido, "REANUDAR", 1) == 0){
			log_info(logger,"PLANIFICACION REANUDADA !!!: ");
			sem_post(&detenerReaunudarEjecucion);
		}
		else if(strncmp(leido, "LISTAR_TRIP", 1) == 0){
			log_info(logger,"LISTA TRIPULANTES----------------------: ");
			hilo_mostrar_tripulantes();
		}
		else if(strncmp(leido, "INICIAR", 1) == 0){
			log_info(logger,"PLANIFICACION INICIADA !!!: ");
			sem_post(&iniciar_planificacion);
		}else if(strncmp(leido, "CREAR_PATOTA", 1) == 0){
			log_info(logger,"CARGAR DATOS PATOTA: ");
			crear_patota();
		}else{
			log_info(logger,"COMANDO INVALIDO");
		}
		leido = readline(">");
	}
	free(leido);
}

void hilo_mostrar_tripulantes(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) listar_tripulantes,NULL);
}


void listar_tripulantes(){

	log_info(logger,"Estado de la Nave");

	pthread_mutex_lock(&planificacion_mutex_new);
	mostrar_lista_tripulantes_new();
	pthread_mutex_unlock(&planificacion_mutex_new);

	pthread_mutex_lock(&planificacion_mutex_ready);
	mostrar_lista_tripulantes_ready();
	pthread_mutex_unlock(&planificacion_mutex_ready);

	pthread_mutex_lock(&planificacion_mutex_bloq);
	mostrar_lista_tripulantes_bloq();
	pthread_mutex_unlock(&planificacion_mutex_bloq);

	pthread_mutex_lock(&mutex_cola_ejecutados);
	mostrar_lista_tripulantes_exec();
	pthread_mutex_unlock(&mutex_cola_ejecutados);

	pthread_mutex_lock(&planificacion_mutex_fin);
	mostrar_lista_tripulantes_fin();
	pthread_mutex_unlock(&planificacion_mutex_fin);
}


void mostrar_lista_tripulantes_new(){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status NEW", tripulante->id,tripulante->patota_id);
	}
	list_iterate(planificacion_cola_new->elements, (void*) mostrar_patota);
}

void mostrar_lista_tripulantes_fin(){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status FIN", tripulante->id,tripulante->patota_id);
	}
	list_iterate(planificacion_cola_fin->elements, (void*) mostrar_patota);
}

void mostrar_lista_tripulantes_bloq(){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status BLOCK I/O", tripulante->id,tripulante->patota_id);
	}
	list_iterate(planificacion_cola_bloq->elements, (void*) mostrar_patota);
}

void mostrar_lista_tripulantes_ready(){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status READY", tripulante->id,tripulante->patota_id);
	}
	list_iterate(planificacion_cola_ready->elements, (void*) mostrar_patota);
}

void mostrar_lista_tripulantes_exec(){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status EXEC", tripulante->id,tripulante->patota_id);
	}
	list_iterate(lista_exec, (void*) mostrar_patota);
}
