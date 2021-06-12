/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils.h"


void iniciar_configuracion(){
	t_config* config = leer_config();

	ip_mongo = config_get_string_value(config, "IP_MONGO");

	ip_miram = config_get_string_value(config, "IP_MIRAM");

	puerto_mongo = config_get_int_value(config,"PUERTO_MONGO_STORE");

	puerto_miram = config_get_int_value(config,"PUERTO_MIRAM");
}

void iniciarEstructurasAdministrativasPlanificador(){
	log_info(logger, "GENERANDO ESTRUCTURAS ADMINISTRATIVAS!");

	sem_init(&iniciar_planificacion, 0, 0);

	sem_init(&detenerReaunudarEjecucion, 0, 1);
	sem_init(&activar_actualizaciones_mongo, 0, 0);

	sem_init(&cola_new, 0, 0);
	sem_init(&cola_ready, 0, 0);
	sem_init(&exec, 0, 1);

	pthread_mutex_init(&planificacion_mutex_new,NULL);
	pthread_mutex_init(&planificacion_mutex_ready,NULL);


	pthread_mutex_init(&comuni,NULL);

	planificacion_cola_new = queue_create();
	planificacion_cola_ready = queue_create();

	hilosParaConexiones = list_create();

	pthread_mutex_init(&mutexHilos,NULL);
}


int terminar_programa(t_log* logger,t_config* config,int conexion[2]) {
	log_destroy(logger);
	config_destroy(config);
	return 0;
}

void iniciar_logger() {
	if( (logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO))==NULL){
		printf("No se pudo crear el logger. Revise parametros\n");
		exit(1);
	}
}

t_config* leer_config() {
	t_config *config;
	if((config = config_create("discordiador.config"))==NULL) {
		printf("No se pudo leer de la config. Revise. \n");
		exit(1);
	}
	return config;
}



