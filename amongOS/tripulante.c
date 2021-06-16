#include "tripulante.h"

char* pedir_tarea(int socketRam, t_tripulante* tripulante) {
	//Lo pasamos a Ready
	sendDeNotificacion(socketRam, PEDIR_TAREA);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	log_info(logger, "tripulante: %d pidio tareas a miram...", tripulante->id);
	uint32_t OPERACION = recvDeNotificacion(socketRam);
	log_info(logger, "OPERACION %d", OPERACION);
	char* tarea = string_new();
	if (OPERACION == ENVIAR_TAREA) {
		tarea = recibirString(socketRam);
		if (tarea != NULL) {
			log_info(logger,
					"tripulante: %d recibio tarea: %s de miram...cantidad tareas: %d",
					tripulante->id, tarea, tripulante->cantidad_tareas);
		}
	}
	return tarea;
}

void actualizar_estado(int socketRam, t_tripulante* tripulante,int estado) {
	//0 new - 1 ready - 2 block - 3 exec - 4 fin
	sendDeNotificacion(socketRam, ACTUALIZAR_ESTADO_MIRAM);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	sendDeNotificacion(socketRam,estado);

	log_info(logger, "tripulante: %d pidio tareas a miram...", tripulante->id);
	uint32_t RESPONSE_ACTUALIZACION = recvDeNotificacion(socketRam);
	log_info(logger, "OPERACION %d", RESPONSE_ACTUALIZACION);
	if (RESPONSE_ACTUALIZACION == ESTADO_ACTUALIZADO_MIRAM) {
		log_info(logger,"tripulante: %d ACTUALIZO ESTADO OK",tripulante->id);
	}
}

void actualizar_ubicacion(int socketRam, t_tripulante* tripulante) {

	sendDeNotificacion(socketRam, ACTUALIZAR_UBICACION);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	sendDeNotificacion(socketRam,tripulante->ubi_x);
	sendDeNotificacion(socketRam,tripulante->ubi_y);

	uint32_t RESPONSE_ACTUALIZACION = recvDeNotificacion(socketRam);
	log_info(logger, "OPERACION %d", RESPONSE_ACTUALIZACION);
	if (RESPONSE_ACTUALIZACION == UBICACION_ACTUALIZADA ) {
		log_info(logger,"tripulante: %d ACTUALIZO UBICACION OK",tripulante->id);
	}
}




void *labor_tripulante_new(void * trip){

	t_tripulante * tripulante = (t_tripulante*) trip;

	sem_init(&tripulante->new,0,0);
	sem_init(&tripulante->ready,0,0);
	sem_init(&tripulante->exec,0,0);
	sem_init(&tripulante->bloq,0,0);

	tripulante->instrucciones_ejecutadas = 0;

	tripulante->ubi_x = 0;
	tripulante->ubi_y = 0;

	log_info(logger,"%d ---- %d", tripulante->id,tripulante->patota_id );

	//Agregamos a cola de NEW
	pthread_mutex_lock(&planificacion_mutex_new);
	queue_push(planificacion_cola_new,tripulante);
	//Avisamos que estamos hay alguien en new
	log_info(logger,"METO A NEW TRIPULANTE %d",tripulante->id);
	pthread_mutex_unlock(&planificacion_mutex_new);
	sem_post(&cola_new);

	sem_wait(&detenerReaunudarEjecucion);
	sem_post(&detenerReaunudarEjecucion);


	int socketMongo = conectarAServer(IP_MONGO, PUERTO_MONGO);
	int socketRam = conectarAServer(IP_MIRAM, PUERTO_MIRAM);

	log_info(logger,"tripulante: %d  se conecto con miram...", tripulante->id);

	sendDeNotificacion(socketRam,CREAR_TRIPULANTE);
	sendDeNotificacion(socketRam,tripulante->id);
	sendDeNotificacion(socketRam,tripulante->patota_id);

	int creado = recvDeNotificacion(socketRam);

	if(creado==TRIPULANTE_CREADO){

		log_info(logger,"TRIPULANTE CREADO, id: %d", tripulante->id);
	}
	actualizar_estado(socketRam,tripulante,NEW);

	//para simular la cantidad;
	char* tarea = pedir_tarea(socketRam, tripulante);

	sem_wait(&tripulante->ready);
	actualizar_estado(socketRam,tripulante,READY);

	log_info(logger,"Enviar tarea a IMONGO STORE %s", tarea);

	char* claveNueva = string_new();

	int tareas_pedidas = 0;
	int rafaga = 0;

	string_append(&claveNueva,tarea);

	sem_wait(&tripulante->exec);
	actualizar_estado(socketRam,tripulante,EXEC);
	
	//simular 3 tareas
	while(tarea!="" && tareas_pedidas<=3){
		
		while(tripulante->instrucciones_ejecutadas<tripulante->cantidad_tareas){

			//Aumenta en par X ; impar Y

			sem_wait(&detenerReaunudarEjecucion);
			sem_post(&detenerReaunudarEjecucion);

			tripulante->instrucciones_ejecutadas++;
			rafaga++;

			log_info(logger,"TAREA EJECUTADA CORRECTAMENTE por tripulante: %d ",tripulante->id);


			//La 'tarea 3' es de entrada salida
			if(tripulante->instrucciones_ejecutadas == 2){
				tripulante->estado = 'B';

				pthread_mutex_lock(&mutex_cola_ejecutados);
				queue_push(cola_ejecutados,tripulante);
				pthread_mutex_unlock(&mutex_cola_ejecutados);
				sem_post(&colaEjecutados);
				sem_post(&exec);

				//ACA INVERTIDO PARA QUE ME DIGA EFECTIVAMENTE CUANDO ESTA EN BLOCK
				actualizar_estado(socketRam,tripulante,BLOCK);
				sem_wait(&tripulante->bloq);


				sem_wait(&tripulante->ready);
				actualizar_estado(socketRam,tripulante,READY);

				sem_wait(&tripulante->exec);
				actualizar_estado(socketRam,tripulante,EXEC);

				enviar_tarea_a_ejecutar(socketMongo, tripulante->id, claveNueva);
				recvDeNotificacion(socketMongo);

				rafaga = 0;
			}

			if(strcmp(ALGORITMO,"RR")==0 && rafaga>=QUANTUM){
				tripulante->estado = 'R';

				pthread_mutex_lock(&mutex_cola_ejecutados);
				queue_push(cola_ejecutados,tripulante);
				pthread_mutex_unlock(&mutex_cola_ejecutados);
				sem_post(&colaEjecutados);
				sem_post(&exec);
				sem_wait(&tripulante->exec);
				actualizar_estado(socketRam,tripulante,EXEC);


				sleep(CICLO_CPU);

				rafaga = 0;
			}

			//Instruccion no es de entrada salida && no es de espera -> actualizo ubicacion
			if(tripulante->instrucciones_ejecutadas!=2){

				if(tripulante->instrucciones_ejecutadas %2 ==0 ){
					tripulante->ubi_x++;
				}else{
					tripulante->ubi_y++;
				}

				actualizar_ubicacion(socketRam,tripulante);
			}


		}
		//Fin tarea

		//Pido Tarea Siguiente
		tarea = pedir_tarea(socketRam, tripulante);
		tripulante->instrucciones_ejecutadas = 0;
		tareas_pedidas++;
	
	}

	tripulante->estado = 'F';
	pthread_mutex_lock(&mutex_cola_ejecutados);
	queue_push(cola_ejecutados,tripulante);
	pthread_mutex_unlock(&mutex_cola_ejecutados);
	sem_post(&colaEjecutados);
	sem_post(&exec);
	actualizar_estado(socketRam,tripulante,FIN);
	log_info(logger, "FIN TAREAS TRIPULANTE %d", tripulante->id);

	//while(1){}
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

