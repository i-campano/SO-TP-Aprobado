/*
 * hilosDiscordiador.c
 *
 *  Created on: 7 jun. 2021
 *      Author: utnso
 */

#include "hilosDiscordiador.h"


void planificar_tripulantes(){
	sem_wait(&iniciar_planificacion);
	hilo_cola_ready();
	hilo_cola_exec();
	hilo_cola_bloq();
	hilo_cola_replanificar();
}

void planificar_cola_ready(){
	while(1){


		//Esperamos que haya algo en new
		sem_wait(&cola_new);

		pthread_mutex_lock(&planificacion_mutex_new);
		t_tripulante * tripulante = queue_pop(planificacion_cola_new);
		log_info(logger,"SACO de NEW tripu: %d", tripulante->id);
		pthread_mutex_unlock(&planificacion_mutex_new);
		sleep(CICLO_CPU);
		pthread_mutex_lock(&planificacion_mutex_ready);
		queue_push(planificacion_cola_ready,tripulante);
		sem_post(&tripulante->ready);
		sem_post(&cola_ready);
		log_info(logger,"METO EN READY tripu: %d", tripulante->id);
		pthread_mutex_unlock(&planificacion_mutex_ready);

		//wait(planificacion_mutex_ready);
		//hacer algo en la cola ready
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);

	}
}

void planificar_cola_bloq(){
	t_tripulante * tripulante;
	while(1){
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);

		sem_wait(&cola_bloq);

		sleep(CICLO_IO);

		pthread_mutex_lock(&planificacion_mutex_bloq);
		tripulante = queue_pop(planificacion_cola_bloq);
		sem_post(&tripulante->bloq);
		log_info(logger,"SACO DE BLOQ TRIPULANTE %d",tripulante->id);
		pthread_mutex_unlock(&planificacion_mutex_bloq);

		pthread_mutex_lock(&planificacion_mutex_ready);
		queue_push(planificacion_cola_ready,tripulante);
		log_info(logger,"METO EN READY TRIPULANTE %d",tripulante->id);
		sem_post(&cola_ready);
		sem_post(&tripulante->ready);
		pthread_mutex_unlock(&planificacion_mutex_ready);

		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);



	}
}




void planificar_cola_exec(){
	while(1){
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);
		sem_wait(&cola_ready);
		sem_wait(&exec);


		pthread_mutex_lock(&planificacion_mutex_ready);
		t_tripulante * tripulante = queue_pop(planificacion_cola_ready);
		log_info(logger,"SACO de READY tripu: %d", tripulante->id);
		pthread_mutex_unlock(&planificacion_mutex_ready);


		sem_wait(&cola_exec);
		pthread_mutex_lock(&planificacion_mutex_exec);
		//queue_push(planificacion_cola_ready,tripulante);
		list_add(lista_exec,tripulante);
		log_info(logger,"METO a EXEC tripu: %d", tripulante->id);
		pthread_mutex_unlock(&planificacion_mutex_exec);


		sem_post(&tripulante->exec);

		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);


	}
}





void replanificar(){
	t_tripulante * tripulante;
	while(1){

		sem_wait(&colaEjecutados);


		pthread_mutex_lock(&mutex_cola_ejecutados);
		tripulante = queue_pop(cola_ejecutados);
		pthread_mutex_unlock(&mutex_cola_ejecutados);

		sacar_de_exec(tripulante->id);
		sem_post(&cola_exec);



		log_info(logger,"DISPATCHER - TRIPULANTE: %d , ESTADO: %c", tripulante->id, tripulante->estado);
		switch(tripulante->estado) {
			case 'F':{
				//MUTEX COLA FIN LOCK
				pthread_mutex_lock(&planificacion_mutex_fin);
				queue_push(planificacion_cola_fin,tripulante);
				log_info(logger,"METO en FIN tripu: %d", tripulante->id);
				pthread_mutex_unlock(&planificacion_mutex_fin);
				sem_post(&cola_fin);

				//MUTEX COLA FIN UNLOCK
				break;
			}
			case 'B':{
				//MUTEX COLA BLOQ LOCK
				pthread_mutex_lock(&planificacion_mutex_bloq);
				queue_push(planificacion_cola_bloq,tripulante);
				log_info(logger,"METO en BLOQ tripu: %d", tripulante->id);
				pthread_mutex_unlock(&planificacion_mutex_bloq);
				sem_post(&cola_bloq);
				break;
				//MUTEX COLA BLOQ UNLO
			}
			case 'R':{
				pthread_mutex_lock(&planificacion_mutex_ready);
				queue_push(planificacion_cola_ready,tripulante);
				log_info(logger,"METO en READY tripu: %d", tripulante->id);
				pthread_mutex_unlock(&planificacion_mutex_ready);
				sem_post(&cola_ready);
				break;
			}
			default:
				break;

		}
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);
	}
}

void sacar_de_exec(int id_tripulante){

	bool encontrarTripulante(t_tripulante * tripulante){
		return tripulante->id == id_tripulante;
	}
	pthread_mutex_lock(&planificacion_mutex_exec);
	t_tripulante * data = (t_tripulante*) list_remove_by_condition(lista_exec,(void*) encontrarTripulante);
	log_info(logger,"SACO de EXEC tripu: %d", data->id);
	pthread_mutex_unlock(&planificacion_mutex_exec);

//	if(data == NULL){
//		log_error(logger, "No se encontro el tripulante %d en la lista de exec", id_tripulante);
//	} else{
//		log_warning(logger, "Tripulante %d salio de exec", id_tripulante);
//
//	}
}


void planif_cola_exec(){
	while(1){
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);

		sem_wait(&cola_ready);
		sem_wait(&exec);

		pthread_mutex_lock(&planificacion_mutex_ready);
		t_tripulante * tripulante = queue_pop(planificacion_cola_ready);
		log_info(logger,"SACO de READY tripu: %d", tripulante->id);
		sem_post(&tripulante->exec);
		pthread_mutex_unlock(&planificacion_mutex_ready);

	}
}



void hilo_cola_exec(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) planificar_cola_exec,NULL);

}
void hilo_cola_replanificar(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) replanificar,NULL);

}




void hilo_cola_ready(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) planificar_cola_ready,NULL);

}


void hilo_cola_bloq(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) planificar_cola_bloq,NULL);
}






void atender_imongo_store(){
	uint32_t notificacion;
	while(1){
		sem_wait(&activar_actualizaciones_mongo);
		sem_post(&activar_actualizaciones_mongo);
		notificacion = recibirUint(socketServerIMongoStore);

		switch(notificacion){
			case ACTUALIZACION_IMONGOSTORE:{
				log_info(logger,"CONEXION VIVA IMONGO CON DISCORDIADOR (SLEEP 10 SEGS)");
			}
		}
	}
}


void atender_ram(){
	uint32_t notificacion;
	while(1){

		notificacion = recibirUint(socketServerMiRam);

		switch(notificacion){
			case PATOTA_CREADA:{


				int * patotaNew = (int *) queue_pop(planificacion_cola_new);

				log_info(logger,"PATOTA ID: %d - CARGADA EN MIRAM", *patotaNew);
				log_info(logger,"PATOTA ID: %d - MOVEMOS DE NEW A READY",*patotaNew);

				//patota patota = malloc(sizeof(patota));


				queue_push(planificacion_cola_ready, patotaNew);

				int * patotaReady = (int *) queue_pop(planificacion_cola_ready);

				log_info(logger,"PATOTA ID: %d  - EN READY", *patotaReady);
			}
		}
	}
}



void iniciarHiloConsola(){
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);
	pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloConsola , &attr2,(void*) leer_consola,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloConsola;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}

void crearHiloTripulante(t_tripulante * tripulante){
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);
	pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
	pthread_t hiloTripulante = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hiloTripulante , &attr2,(void*) labor_tripulante_new,(void*) tripulante);

//	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
//	datosHilo->socket = socket;
//	datosHilo->hiloAtendedor = hiloTripulante;
//
//	pthread_mutex_lock(&mutexHilos);
//	list_add(hilosParaConexiones, datosHilo);
//	pthread_mutex_unlock(&mutexHilos);
}

void planificar(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloPlanificador , &attr1,(void*) planificar_tripulantes,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloPlanificador;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}


void atenderLaRam(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloConsola , &attr1,(void*) atender_ram,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloConsola;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}

void atenderIMongoStore(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloConsola , &attr1,(void*) atender_imongo_store,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloConsola;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}
