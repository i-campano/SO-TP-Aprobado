#include "tripulante.h"

char* pedir_tarea(int socketRam, t_tripulante* tripulante) {
	//Lo pasamos a Ready
	sendDeNotificacion(socketRam, PEDIR_TAREA);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	log_debug(logger,"T%d - P%d : PIDIO TAREA", tripulante->id,tripulante->patota_id);
	uint32_t OPERACION = recvDeNotificacion(socketRam);
//	log_info(logger, "OPERACION %d", OPERACION);
	char* tarea = string_new();
	if (OPERACION == ENVIAR_TAREA) {
		tarea = recibirString(socketRam);
		if (tarea != NULL) {
			log_debug(logger,"T%d - P%d : RECIBIO TAREA", tripulante->id,tripulante->patota_id);
		}
	}
	return tarea;
}

void actualizar_estado(int socketRam, t_tripulante* tripulante,int estado) {
	//0 new - 1 ready - 2 block - 3 exec - 4 fin
	sendDeNotificacion(socketRam, ACTUALIZAR_ESTADO_MIRAM);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	sendDeNotificacion(socketRam,estado);

	uint32_t RESPONSE_ACTUALIZACION = recvDeNotificacion(socketRam);
//	log_info(logger, "OPERACION %d", RESPONSE_ACTUALIZACION);
	if (RESPONSE_ACTUALIZACION == ESTADO_ACTUALIZADO_MIRAM) {
		log_debug(logger,"T%d - P%d : ACTUALIZO ESTADO OK", tripulante->id,tripulante->patota_id);
	}
}

void actualizar_ubicacion(int socketRam, t_tripulante* tripulante) {

	sendDeNotificacion(socketRam, ACTUALIZAR_UBICACION);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	sendDeNotificacion(socketRam,tripulante->ubi_x);
	sendDeNotificacion(socketRam,tripulante->ubi_y);

	uint32_t RESPONSE_ACTUALIZACION = recvDeNotificacion(socketRam);

	if (RESPONSE_ACTUALIZACION == UBICACION_ACTUALIZADA ) {
		log_debug(logger,"T%d - P%d : ACTUALIZO UBICACION OK", tripulante->id,tripulante->patota_id);
	}
}



//	ej de carga por consola: GENERAR_OXIGENO 12;2;3;5-REGAR_PLANTAS;2;3;5
char* parsear_tarea(char* tarea,int* movX,int* movY,int* esIo,int* tiempo_tarea, int * cpuBound) {
	char** tarea_separada = string_split(tarea,";");
	char** tarea_parametro = string_split(tarea_separada[0]," ");

	*movX = strtol(tarea_separada[1], NULL, 10);
	*movY = strtol(tarea_separada[2], NULL, 10);
	*tiempo_tarea = strtol(tarea_separada[3], NULL, 10);

	*esIo = 1;
	*cpuBound = *movX + *movY;
	if(tarea_parametro[1] == NULL) {
		*esIo = 0;
	}

	log_debug(logger,"PARSER: tarea: %s - movX: %d - movY: %d - esIo: %d - tiempo_tarea: %d",tarea_separada[0], *movX,*movY,*esIo,*tiempo_tarea);
	free(tarea_parametro);//stringsplit
	return tarea_separada[0];
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
	log_info(logger,"T%d - P%d : NEW", tripulante->id,tripulante->patota_id);
	pthread_mutex_unlock(&planificacion_mutex_new);
	sem_post(&cola_new);

	sem_wait(&detenerReaunudarEjecucion);
	sem_post(&detenerReaunudarEjecucion);


	int socketMongo = conectarAServer(IP_MONGO, PUERTO_MONGO);
	int socketRam = conectarAServer(IP_MIRAM, PUERTO_MIRAM);

	log_debug(logger,"T%d - P%d : CONEXION MIRAM OK", tripulante->id,tripulante->patota_id);

	sendDeNotificacion(socketRam,CREAR_TRIPULANTE);
	sendDeNotificacion(socketRam,tripulante->id);
	sendDeNotificacion(socketRam,tripulante->patota_id);

	int creado = recvDeNotificacion(socketRam);

	if(creado==TRIPULANTE_CREADO){

		log_debug(logger,"T%d - P%d : TRIPULANTE CREADO OK", tripulante->id,tripulante->patota_id);
	}
	actualizar_estado(socketRam,tripulante,NEW);

	//para simular la cantidad;
	char* tarea = pedir_tarea(socketRam, tripulante);
	int movX = 0 ;
	int movY= 0;
	int esIo = 0;
	int tiempo_tarea = 0;
	int cpuBound = 0;
	parsear_tarea(tarea,&movX,&movY,&esIo,&tiempo_tarea,&cpuBound);

	sem_wait(&tripulante->ready);
	actualizar_estado(socketRam,tripulante,READY);


	char* claveNueva = string_new();

	int tareas_pedidas = 0;
	int rafaga = 0;

	string_append(&claveNueva,tarea);

	sem_wait(&tripulante->exec);
	actualizar_estado(socketRam,tripulante,EXEC);
	while(strcmp(tarea,"--")!=0){

		log_info(logger,"T%d - P%d : COMIENZA A EJECUTAR: %s", tripulante->id,tripulante->patota_id, tarea);
		
		while(tripulante->instrucciones_ejecutadas<cpuBound+tiempo_tarea){

			//Aumenta en par X ; impar Y

			sem_wait(&detenerReaunudarEjecucion);
			sem_post(&detenerReaunudarEjecucion);

			tripulante->instrucciones_ejecutadas++;
			rafaga++;


			sleep(CICLO_CPU);

			//La 'tarea 3' es de entrada salida
			if(esIo && tripulante->instrucciones_ejecutadas>movX+movY){

				log_info(logger," 															********	T%d - P%d :	IO BOUND    *****", tripulante->id,tripulante->patota_id);
				tripulante->estado = 'B';

				pthread_mutex_lock(&mutex_cola_ejecutados);
				queue_push(cola_ejecutados,tripulante);
				pthread_mutex_unlock(&mutex_cola_ejecutados);
				sem_post(&colaEjecutados);
				sem_post(&exec);

				//ACA INVERTIDO PARA QUE ME DIGA EFECTIVAMENTE CUANDO ESTA EN BLOCK
				actualizar_estado(socketRam,tripulante,BLOCK);

				tripulante->block_io_rafaga = tiempo_tarea;
				sem_wait(&tripulante->bloq);

				enviar_tarea_a_ejecutar(socketMongo, tripulante->id, claveNueva);
				recvDeNotificacion(socketMongo);

				tripulante->instrucciones_ejecutadas+=tiempo_tarea;



				sem_wait(&tripulante->ready);
				actualizar_estado(socketRam,tripulante,READY);

				sem_wait(&tripulante->exec);
				actualizar_estado(socketRam,tripulante,EXEC);



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


				rafaga = 0;
			}


			if(!(esIo && tripulante->instrucciones_ejecutadas>movX+movY)){
				log_info(logger," 															++++++++   	T%d - P%d :	CPU BOUND     +++++++", tripulante->id,tripulante->patota_id);
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

			log_info(logger,"CICLO TERMINADO por tripulante: %d ",tripulante->id);


		}
		//Fin tarea


			log_info(logger,"PEDIR PROXIMA TAREA por tripulante: %d ",tripulante->id);
			tarea = pedir_tarea(socketRam, tripulante);
			if(strcmp(tarea,"--")!=0){
				parsear_tarea(tarea,&movX,&movY,&esIo,&tiempo_tarea,&cpuBound);
				tripulante->instrucciones_ejecutadas = 0;
				tripulante->cantidad_tareas--;

				//break;
			}
		//Pido Tarea Siguiente

	
	}

	tripulante->estado = 'F';
	pthread_mutex_lock(&mutex_cola_ejecutados);
	queue_push(cola_ejecutados,tripulante);
	pthread_mutex_unlock(&mutex_cola_ejecutados);
	sem_post(&colaEjecutados);
	sem_post(&exec);
	actualizar_estado(socketRam,tripulante,FIN);
	log_info(logger, "FIN TAREAS TRIPULANTE %d", tripulante->id);
	free(claveNueva);
	//while(1){}
	return 0; //Para que no moleste el warning
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
	free(buffer); //Malloc linea 253
}

