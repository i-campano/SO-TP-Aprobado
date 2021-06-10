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

	hilo_cola_new();
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

	sem_wait(&iniciar_cola_ready);
	while(1){
		sleep(3);
		sem_wait(&sistemaEnEjecucion);
		sem_post(&sistemaEnEjecucion);
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
	while(1){
		sleep(6);
		sem_wait(&sistemaEnEjecucion);
		sem_post(&sistemaEnEjecucion);
		log_info(logger,"PLANIFICANDOO COLA NEW");
	}
}
