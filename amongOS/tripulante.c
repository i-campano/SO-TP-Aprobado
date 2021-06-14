#include "tripulante.h"

void *labor_tripulante_new(void * trip){
	//¿ estructura estatica dentro del hilo? --- pensar
	//add a lista de sem ??

	t_tripulante * tripulante = (t_tripulante*) trip;

	tripulante->instrucciones_ejecutadas = 0;

	sem_init(&tripulante->new,0,0);
	sem_init(&tripulante->ready,0,0);
	sem_init(&tripulante->exec,0,0);

	tripulante->fin = 0;

	pthread_mutex_init(&tripulante->ejecutadas,NULL);

	log_info(logger,"%d ---- %d", tripulante->id,tripulante->patota_id );

	//Agregamos a cola de NEW
	pthread_mutex_lock(&planificacion_mutex_new);
	queue_push(planificacion_cola_new,tripulante);
	//Avisamos que estamos hay alguien en new
	sem_post(&cola_new);
	pthread_mutex_unlock(&planificacion_mutex_new);

	log_info(logger,"AGREGUE A LA COLA DE NEW");

	int socketMongo = conectarAServer(ip_mongo, puerto_mongo);

	int socketRam = conectarAServer(ip_miram, puerto_miram);
	log_info(logger,"tripulante: %d  se conecto con miram...", tripulante->id);

	sendDeNotificacion(socketRam,CREAR_TRIPULANTE);


	int creado = recvDeNotificacion(socketRam);

	if(creado==TRIPULANTE_CREADO){

		log_info(logger,"TRIPULANTE CREADO, id: %d", tripulante->id);
	}

	//Lo pasamos a Ready


	sendDeNotificacion(socketRam, PEDIR_TAREA);

	sendDeNotificacion(socketRam,(uint32_t)tripulante->id);
	log_info(logger,"tripulante: %d pidio tareas a miram...", tripulante->id);

	uint32_t OPERACION = recvDeNotificacion(socketRam);

	log_info(logger,"OPERACION %d",OPERACION);
	char * tarea = string_new();
	if(OPERACION==ENVIAR_TAREA){
		tarea = recibirString(socketRam);
		if(tarea!=NULL){
			log_info(logger,"tripulante: %d recibio tarea: %s de miram...cantidad tareas: %d", tripulante->id,tarea,tripulante->cantidad_tareas);
		}

	}
	sem_post(&tripulante->ready);



	log_info(logger,"Enviar tarea a IMONGO STORE %s", tarea);


	char* claveNueva = string_new();

	string_append(&claveNueva,tarea);
	sem_wait(&tripulante->exec);
	while(tripulante->instrucciones_ejecutadas<tripulante->cantidad_tareas){

		sem_wait(&detenerReaunudarEjecucion);
		sem_post(&detenerReaunudarEjecucion);


		//tripulante->estado = B | FQ | FIN


		tripulante->instrucciones_ejecutadas++;


		enviar_tarea_a_ejecutar(socketMongo, tripulante->id, claveNueva);
		recvDeNotificacion(socketMongo);
		sleep(2);
		log_info(logger,"TAREA EJECUTADA CORRECTAMENTE por tripulante: %d ",tripulante->id);


	}
	tripulante->estado = 'F';
	pthread_mutex_lock(&mutex_cola_ejecutados);
	queue_push(cola_ejecutados,tripulante);
	pthread_mutex_unlock(&mutex_cola_ejecutados);
	sem_post(&colaEjecutados);
	sem_post(&exec);

	while(1){}

	log_info(logger, "FIN TAREAS TRIPULANTE %d", tripulante->id);
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

