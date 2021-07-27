/*
 * consolaDiscordiador.c
 *
 *  Created on: 14 jun. 2021
 *      Author: utnso
 */

#include "consolaDiscordiador.h"
int programa_ejecucion = 1;
void leer_consola() {
	log_info(logger,"INGRESE UN COMANDO: ");

	char* leido = readline(">");
	while(strncmp(leido, "TERM", 4) != 0) {
		log_info(logger, leido);

		if(strncmp(leido, "DETENER", 2) == 0){
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
		else if(strncmp(leido, "PLANIFICACION", 5) == 0){
			log_info(logger,"PLANIFICACION INICIADA !!!: ");
			sem_post(&iniciar_planificacion);
		}else if(strncmp(leido, "INICIAR_PATOTA", 14) == 0){
			//EJEMPLO COMANDO: INICIAR_PATOTA oxigeno.txt 2 12 1|2
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 1 1 6|2
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 2 13 6|2 3|5
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 2 14 1|2 4|5
			//EJEMPLO COMANDO: INICIAR_PATOTA oxigeno.txt 2 15 5|2 3|6

			//EJEMPLO COMANDO: INICIAR_PATOTA pag_a.txt 1 1 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA pag_b.txt 1 2 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA pag_c.txt 1 3 0|0

			//EJEMPLO COMANDO: INICIAR_PATOTA tareasLargas.txt 1 1 0|0

			//EJEMPLO COMANDO: INICIAR_PATOTA segA.txt 4 1 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA segB.txt 2 2 0|0
			//EJEMPLO COMANDO: INICIAR_PATOTA segC.txt 1 3 0|0

			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 2 15 5|2 3|6

			crear_patota(leido);
		}
		else if(strncmp(leido, "GET_DATOS", 3) == 0){
			sendDeNotificacion(socketServerMiRam, GET_PCB);
			sendDeNotificacion(socketServerMiRam,1);
		}
		else if(strncmp(leido, "DUMP", 3) == 0){
			sendDeNotificacion(socketServerMiRam, DUMP);
		}
		else if(strncmp(leido, "COMPACTAR", 3) == 0){
			sendDeNotificacion(socketServerMiRam, COMPACTACION);
		}
		else if(strncmp(leido, "EXPULSAR_TRIPULANTE", 9) == 0){
			char** separado = string_split(leido," ");
			sendDeNotificacion(socketServerMiRam, EXPULSAR_TRIPULANTE);
			t_tripulante* trip = buscarTripulante(atoi(separado[1]));
			sendDeNotificacion(socketServerMiRam,trip->id);
			sendDeNotificacion(socketServerMiRam,trip->patota_id);
			sendDeNotificacion(socketServerMiRam,trip->direccionLogica);
		}
		else{
			log_info(logger,"COMANDO INVALIDO");
		}
		leido = readline(">");
	}
	sem_post(&terminarPrograma);
	free(leido);
}


void escuchoSabotaje() {
	log_info(logger,"Atendedor de sabotajes: Escuchando OK");
	while(1) {
		uint32_t nroNotificacion = recvDeNotificacion(socketServerIMongoStore);
		log_info(logger,"LLEGO UN MENSAJE DEL IMONGO");
		if(nroNotificacion==INFORMAR_SABOTAJE){

			char * posicion = recibirString(socketServerIMongoStore);

			sendDeNotificacion(socketServerIMongoStore,FSCK);

			log_info(logger,"Llego un SABOTAJE en la posicion: %s",posicion);
		}
	}

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
	mostrar_lista_tripulantes(planificacion_cola_new,"NEW");
	pthread_mutex_unlock(&planificacion_mutex_new);

	pthread_mutex_lock(&planificacion_mutex_ready);
	mostrar_lista_tripulantes(planificacion_cola_ready,"READY");
	pthread_mutex_unlock(&planificacion_mutex_ready);

	pthread_mutex_lock(&planificacion_mutex_bloq);
	mostrar_lista_tripulantes(planificacion_cola_bloq,"BLOQ");
	pthread_mutex_unlock(&planificacion_mutex_bloq);

	pthread_mutex_lock(&mutex_cola_ejecutados);
	mostrar_lista_tripulantes_exec();
	pthread_mutex_unlock(&mutex_cola_ejecutados);

	pthread_mutex_lock(&planificacion_mutex_fin);
	mostrar_lista_tripulantes(planificacion_cola_fin,"FIN");
	pthread_mutex_unlock(&planificacion_mutex_fin);
}


void mostrar_lista_tripulantes(t_queue* queue,char * nombre_cola){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status: %s", tripulante->id,tripulante->patota_id,nombre_cola);
	}
	list_iterate(queue->elements, (void*) mostrar_patota);
}

void mostrar_lista_tripulantes_exec(){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,"Tripulante: %d   Patota: %d    Status EXEC", tripulante->id,tripulante->patota_id);
	}
	list_iterate(lista_exec, (void*) mostrar_patota);
}
t_tripulante* buscarTripulante(uint32_t id_trip){
	bool condicionId(void* dato){
		t_tripulante* trip = (t_tripulante*)dato;
		return trip->id == id_trip;
	}
	pthread_mutex_lock(&planificacion_mutex_new);
	t_tripulante* trip = list_find(planificacion_cola_new->elements,condicionId);
	if(trip != NULL){
		pthread_mutex_lock(&planificacion_mutex_fin);
		queue_push(planificacion_cola_fin,list_remove_by_condition(planificacion_cola_new->elements,condicionId));
		pthread_mutex_unlock(&planificacion_mutex_fin);
	}
	pthread_mutex_unlock(&planificacion_mutex_new);
	if(trip != NULL){
		return trip;
	}
	pthread_mutex_lock(&planificacion_mutex_ready);
	trip = list_find(planificacion_cola_ready->elements,condicionId);
	if(trip != NULL){
		pthread_mutex_lock(&planificacion_mutex_fin);
		queue_push(planificacion_cola_fin,list_remove_by_condition(planificacion_cola_ready->elements,condicionId));
		pthread_mutex_unlock(&planificacion_mutex_fin);
	}
	pthread_mutex_unlock(&planificacion_mutex_ready);
	if(trip != NULL){
		return trip;
	}
	pthread_mutex_lock(&mutex_cola_ejecutados);
	trip = list_find(lista_exec,condicionId);
	if(trip != NULL){
		pthread_mutex_lock(&planificacion_mutex_fin);
		queue_push(planificacion_cola_fin,list_remove_by_condition(lista_exec,condicionId));
		pthread_mutex_unlock(&planificacion_mutex_fin);
	}
	pthread_mutex_unlock(&mutex_cola_ejecutados);
	if(trip != NULL){
		return trip;
	}
	pthread_mutex_lock(&planificacion_mutex_bloq);
	trip = list_find(planificacion_cola_bloq->elements,condicionId);
	if(trip != NULL){
		pthread_mutex_lock(&planificacion_mutex_fin);
		queue_push(planificacion_cola_fin,list_remove_by_condition(planificacion_cola_bloq->elements,condicionId));
		pthread_mutex_unlock(&planificacion_mutex_fin);
	}
	pthread_mutex_unlock(&planificacion_mutex_bloq);
	if(trip != NULL){
		return trip;
	}
	pthread_mutex_lock(&planificacion_mutex_fin);
	trip = list_find(planificacion_cola_fin->elements,condicionId);
	pthread_mutex_unlock(&planificacion_mutex_fin);
	if(trip != NULL){
		return trip;
	}
	return NULL;
}
