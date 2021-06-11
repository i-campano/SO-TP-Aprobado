/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils.h"


void iniciarEstructurasAdministrativasPlanificador(){
	log_info(logger, "GENERANDO ESTRUCTURAS ADMINISTRATIVAS!");


	sem_init(&cola_new, 0, 0);
	sem_init(&cola_ready, 0, 0);
	sem_init(&iniciar_planificacion, 0, 0);
	sem_init(&iniciar_cola_ready, 0, 0);
	sem_init(&sistemaEnEjecucion, 0, 1);
	sem_init(&activar_actualizaciones_mongo, 0, 0);
	sem_init(&exec, 0, 1);

	pthread_mutex_init(&planificacion_mutex_new,NULL);
	pthread_mutex_init(&planificacion_mutex_ready,NULL);


	pthread_mutex_init(&comuni,NULL);

	planificacion_cola_new = queue_create();
	planificacion_cola_ready = queue_create();

	hilosParaConexiones = list_create();

	pthread_mutex_init(&mutexHilos,NULL);
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
				sem_post(&iniciar_cola_ready);
			}
		}
	}
}
