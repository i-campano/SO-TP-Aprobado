/*
 * hilosDiscordiador.c
 *
 *  Created on: 7 jun. 2021
 *      Author: utnso
 */

#include "hilosDiscordiador.h"

void leer_consola() {
	log_info(logger,"INGRESE UN COMANDO: ");
	tripulantes_creados = 0;
	char* leido = readline(">");
	while(strncmp(leido, "", 1) != 0) {
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
		else if(strncmp(leido, "XMOSTRAR_NEW", 5) == 0){
			log_info(logger,"TRIPULANTES EN NEW!!!: ");
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

void planificar_tripulantes(){
	sem_wait(&iniciar_planificacion);

	hilo_cola_ready();
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

void planificar_cola_new(){
	log_info(logger,"PLANIFICANDOO COLA NEW");
		while(1){
			sleep(10);
			pthread_mutex_lock(&planificacion_mutex_new);
			mostrar_lista_tripulantes();
			pthread_mutex_unlock(&planificacion_mutex_new);
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
