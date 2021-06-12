#include "tripulante.h"

void *labor_tripulante_new(void * id_tripulante){
	//¿ estructura estatica dentro del hilo? --- pensar
	//add a lista de sem ??

	int id = *(int*)id_tripulante;

	t_tripulante * tripulante = (t_tripulante*) malloc(sizeof(t_tripulante));
	tripulante->id = id;
	sem_init(&tripulante->new,0,0);
	sem_init(&tripulante->ready,0,0);
	sem_init(&tripulante->exec,0,0);


	pthread_mutex_lock(&planificacion_mutex_new);
	queue_push(planificacion_cola_new,tripulante);
	sem_post(&cola_new);
	pthread_mutex_unlock(&planificacion_mutex_new);

	log_info(logger,"AGREGUE A LA COLA DE NEW");



	int socketRam = conectarAServer(ip_miram, puerto_miram);
	log_info(logger,"tripulante: %d  se conecto con miram...", id);

	sendDeNotificacion(socketRam,CREAR_TRIPULANTE);


	int creado = recvDeNotificacion(socketRam);

	if(creado==TRIPULANTE_CREADO){

		log_info(logger,"TRIPULANTE CREADO, id: %d", id);
	}


	//while(tengaTareas)

		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);
		sem_wait(&tripulante->ready);
		//sem_wait(&ready)

		sendDeNotificacion(socketRam, PEDIR_TAREA);

		sendDeNotificacion(socketRam,(uint32_t)id);
		log_info(logger,"tripulante: %d pidio tareas a miram...", id);

		uint32_t OPERACION = recvDeNotificacion(socketRam);

		//sem_wait(&EXEC)

		log_info(logger,"OPERACION %d",OPERACION);
		char * tarea = string_new();
		if(OPERACION==ENVIAR_TAREA){
			tarea = recibirString(socketRam);
			if(tarea!=NULL){

				log_info(logger,"tripulante: %d recibio tarea: %s de miram...", id,tarea);
			}else{
				//break while - cola fin
			}

		}
		sem_wait(&cola_ready);
		sem_wait(&exec);
		log_info(logger,"Enviar tarea a IMONGO STORE %s", tarea);

		int socketMongo = conectarAServer(ip_mongo, puerto_mongo);


		char* claveNueva = string_new();

		string_append(&claveNueva,tarea);

		enviar_tarea_a_ejecutar(socketMongo, id, claveNueva);


		recvDeNotificacion(socketMongo);
		sleep(2);
		log_info(logger,"TAREA EJECUTADA CORRECTAMENTE por tripulante: %d ",id);
		sem_post(&exec);


	//fin WHILE(tengaTareas)
}

void enviar_tarea_a_ejecutar(int socketMongo, int id, char* claveNueva) {
	int largoClave = string_length(claveNueva);
	int tamanio = 0;
	//En el buffer mando clave y luego valor
	void* buffer = malloc(string_length(claveNueva) + sizeof(uint32_t));
	memcpy(buffer + tamanio, &largoClave, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);
	memcpy(buffer + tamanio, claveNueva, string_length(claveNueva));
	tamanio += largoClave;
	sendRemasterizado(socketMongo, EJECUTAR_TAREA, tamanio, (void*) buffer);
	sendDeNotificacion(socketMongo, (uint32_t) id);
}

