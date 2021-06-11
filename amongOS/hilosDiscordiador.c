/*
 * hilosDiscordiador.c
 *
 *  Created on: 7 jun. 2021
 *      Author: utnso
 */

#include "hilosDiscordiador.h"

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


void planificar_tripulantes(){
	sem_wait(&iniciar_planificacion);

	hilo_cola_ready();
}

void hilo_cola_ready(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloColaReady , &attr1,(void*) planificar_cola_ready,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloColaReady;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}

void planificar_cola_ready(){
	while(1){
		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);

		sem_wait(&cola_new);
		pthread_mutex_lock(&planificacion_mutex_new);
		//quizas ademas deberia hacer un consumidor productor para asegurarme que el pop tenga algo
		sleep(3);
		t_tripulante * tripulante = queue_pop(planificacion_cola_new);
		log_info(logger,"saco de new tripu: %d", tripulante->id);
		pthread_mutex_unlock(&planificacion_mutex_new);
		sleep(1);
		pthread_mutex_lock(&planificacion_mutex_ready);
		queue_push(&planificacion_cola_ready,tripulante);
		sem_post(&tripulante->ready);
		sem_post(&cola_ready);
		pthread_mutex_unlock(&planificacion_mutex_ready);

		log_info(logger,"PLANIFICANDOO COLA READY");
		//wait(planificacion_mutex_ready);
		//hacer algo en la cola ready

	}
}


void hilo_cola_new(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloColaReady , &attr1,(void*) planificar_cola_new,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloColaReady;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}


void planificar_cola_new(){
	log_info(logger,"PLANIFICANDOO COLA NEW");
		while(1){
			sleep(10);
			pthread_mutex_lock(&planificacion_mutex_new);
			mostrar_lista_tripulantes();
			pthread_mutex_unlock(&planificacion_mutex_new);

		}

}

void hilo_mostrar_tripulantes(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_create(&hiloColaReady , &attr1,(void*) mostrar_tripulantes_new,NULL);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = 0;
	datosHilo->hiloAtendedor = hiloColaReady;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}

void mostrar_tripulantes_new(){
	sem_wait(&cola_new);
	pthread_mutex_lock(&planificacion_mutex_new);
	mostrar_lista_tripulantes();
	pthread_mutex_unlock(&planificacion_mutex_new);
	sem_post(&cola_new);
}

void mostrar_lista_tripulantes(){
	void mostrar_patota(t_tripulante* tripulante){
		log_info(logger,".........MOSTRANDO TRIPULANTE EN NEW........# N:, id tripulante %d", tripulante->id);
	}
	list_iterate(planificacion_cola_new->elements, (void*) mostrar_patota);
}
