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
		else if(strncmp(leido, "PLANIFICACION", 5) == 0){
			log_info(logger,"PLANIFICACION INICIADA !!!: ");
			sem_post(&iniciar_planificacion);

		}else if(strncmp(leido, "PEDIR_BITACORA", 7) == 0){
				log_info(logger,"PEDIR_BITACORA !!!: ");
				char ** parametros = string_n_split(leido,2," ");
				sendDeNotificacion(socketServerIMongoStore,PEDIR_BITACORA);
				sendDeNotificacion(socketServerIMongoStore,atoi(parametros[1]));

		}else if(strncmp(leido, "INICIAR_PATOTA", 14) == 0){
			//EJEMPLO COMANDO: INICIAR_PATOTA oxigeno.txt 2 12 1|2
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 1 1 6|2
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 2 13 6|2 3|5
			//EJEMPLO COMANDO: INICIAR_PATOTA plantas.txt 2 14 1|2 4|5
			//EJEMPLO COMANDO: INICIAR_PATOTA oxigeno.txt 2 15 5|2 3|6
			//EJEMPLO COMANDO: INICIAR_PATOTA oxigeno.txt 2 16 1|2 |5
			crear_patota(leido);
		}
		else if(strncmp(leido, "GET_DATOS", 3) == 0){
			sendDeNotificacion(socketServerMiRam, GET_PCB);
			sendDeNotificacion(socketServerMiRam,1);
		}
		else{
			log_info(logger,"COMANDO INVALIDO");
		}
		leido = readline(">");
	}
	sem_post(&terminarPrograma);
	free(leido);
}


void escuchoIMongo() {
	log_info(logger,"escuchoIMongo(): Escuchando OK");
	while(1) {
		uint32_t nroNotificacion = recvDeNotificacion(socketServerIMongoStore);
		log_debug(logger,"escuchoIMongo(): Llego una notificacion de iMongo");
		if(nroNotificacion==INFORMAR_SABOTAJE){
			//Recibe notificacion de sabojate
			char * posicion = recibirString(socketServerIMongoStore);

			sendDeNotificacion(socketServerIMongoStore,FSCK);

			log_info(logger,"Llego un SABOTAJE en la posicion: %s",posicion);
		}
		else if(nroNotificacion==ENVIAR_BITACORA){
			//Recibe informacion de bitacora
			char * bitacora = recibirString(socketServerIMongoStore);
			printf("Bitacora:\n%s",bitacora);
		}
		else{
			//Notificacion desconocida -> IMongo caido
			log_debug(logger,"escuchoIMongo(): mensaje desconocido");
			close(socketServerIMongoStore);

			while(1){
				sleep(5);
				log_info(logger,"escuchoIMongo(): Intentando reestablecer la comunicacion con iMongo");
				socketServerIMongoStore = reConectarAServer("127.0.0.1", 5003);
				if(socketServerIMongoStore>0){
					log_info(logger,"escuchoIMongo(): se pudo reconectar");
					break;
				}else
				{
					log_info(logger,"escuchoIMongo(): no se pudo reconectar");
				}
			}
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
