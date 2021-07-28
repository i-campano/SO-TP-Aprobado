#include "tripulante.h"

char* pedir_tarea(int socketRam, t_tripulante* tripulante) {
	//Lo pasamos a Ready
//	log_debug(logger,"T%d - P%d : PIDIO TAREA", tripulante->id,tripulante->patota_id);
	sendDeNotificacion(socketRam, PEDIR_TAREA);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->patota_id);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->direccionLogica);
	uint32_t OPERACION = recvDeNotificacion(socketRam);
//	log_info(logger, "OPERACION %d", OPERACION);
	char* tarea = string_new();
	if (OPERACION == ENVIAR_TAREA) {
		tarea = recibirString(socketRam);
		if (tarea != NULL) {
			log_trace(logger,"T%d - P%d : RECIBIO TAREA", tripulante->id,tripulante->patota_id);
		}
	}
	return tarea;
}

void actualizar_estado(int socketRam, t_tripulante* tripulante,int estado) {
	//0 new - 1 ready - 2 block - 3 exec - 4 fin
	sendDeNotificacion(socketRam, ACTUALIZAR_ESTADO_MIRAM);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->patota_id);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->direccionLogica);
	sendDeNotificacion(socketRam,estado);

	uint32_t RESPONSE_ACTUALIZACION = recvDeNotificacion(socketRam);
//	log_info(logger, "OPERACION %d", RESPONSE_ACTUALIZACION);
	if (RESPONSE_ACTUALIZACION == ESTADO_ACTUALIZADO_MIRAM) {
		log_trace(logger,"T%d - P%d : ACTUALIZO ESTADO OK", tripulante->id,tripulante->patota_id);
	}
}

void actualizar_ubicacion(int socketRam, t_tripulante* tripulante) {

	sendDeNotificacion(socketRam, ACTUALIZAR_UBICACION);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->id);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->patota_id);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->direccionLogica);
	sendDeNotificacion(socketRam,tripulante->ubi_x);
	sendDeNotificacion(socketRam,tripulante->ubi_y);

	uint32_t RESPONSE_ACTUALIZACION = recvDeNotificacion(socketRam);

	if (RESPONSE_ACTUALIZACION == UBICACION_ACTUALIZADA ) {
		log_debug(logger,"T%d - P%d : ACTUALIZO UBICACION OK", tripulante->id,tripulante->patota_id);
	}
}



//	ej de carga por consola: GENERAR_OXIGENO 12;2;3;5-REGAR_PLANTAS;2;3;5
char* parsear_tarea(char* tarea,int* movX,int* movY,int* esIo,int* tiempo_tarea) {
	char** tarea_separada = string_split(tarea,";");
	char** tarea_parametro = string_split(tarea_separada[0]," ");

	*movX = strtol(tarea_separada[1], NULL, 10);
	*movY = strtol(tarea_separada[2], NULL, 10);
	*tiempo_tarea = strtol(tarea_separada[3], NULL, 10);

	*esIo = 1;
	if(tarea_parametro[1] == NULL) {
		*esIo = 0;
	}

	log_trace(logger,"PARSER: tarea: %s - movX: %d - movY: %d - esIo: %d - tiempo_tarea: %d",tarea_separada[0], *movX,*movY,*esIo,*tiempo_tarea);
	free(tarea_parametro);//stringsplit
	return tarea_separada[0];
}

void *labor_tripulante_new(void * trip){

	t_tripulante * tripulante = (t_tripulante*) trip;

	sem_wait(&tripulante->creacion);

	log_info(logger,"Hilo tripulante CREADO: %d de patota: %d DirLog: %i", tripulante->id,tripulante->patota_id,tripulante->direccionLogica);


//	sem_init(&tripulante->emergencia,0,0);
	sem_init(&tripulante->new,0,0);
	sem_init(&tripulante->ready,0,0);
	sem_init(&tripulante->exec,0,0);
	sem_init(&tripulante->bloq,0,0);

//	tripulante->instrucciones_ejecutadas = 0;

	int firstMove = 0;
	int moveUp = 0;
	int moveRight = 0;

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

	list_add(conexiones,socketMongo);
	list_add(conexiones,socketRam);

	log_info(logger,"T%d - P%d : CONEXION MIRAM OK", tripulante->id,tripulante->patota_id);

	//obtener data tripulante - desde tcb

	sendDeNotificacion(socketRam,PEDIR_UBICACION);
	sendDeNotificacion(socketRam,tripulante->id);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->patota_id);
	sendDeNotificacion(socketRam, (uint32_t) tripulante->direccionLogica);
	tripulante->ubi_x = (uint32_t)recvDeNotificacion(socketRam);
	tripulante->ubi_y = (uint32_t)recvDeNotificacion(socketRam);
	log_info(logger,"Hilo tripulante: %d de patota: %d --- x: %d --- y: %d ", tripulante->id,tripulante->patota_id,tripulante->ubi_x,tripulante->ubi_y);
//	sendDeNotificacion(socketRam,tripulante->patota_id);
//
//
//	sendDeNotificacion(socketRam,atoi(coordenadas_posicion_inicial[0]));
//	sendDeNotificacion(socketRam,atoi(coordenadas_posicion_inicial[1]));
//	free(coordenadas_posicion_inicial);
//
//
//
//	if(creado==TRIPULANTE_CREADO){
//
//		log_debug(logger,"T%d - P%d : TRIPULANTE CREADO OK", tripulante->id,tripulante->patota_id);
//	}



//	actualizar_estado(socketRam,tripulante,NEW);
	sem_wait(&tripulante->ready);

	log_info(logger,"Pido la proxima tarea");
	//para simular la cantidad;
	char* tarea = pedir_tarea(socketRam, tripulante);
	log_info(logger,"Recibi tarea %s",tarea);
	int movX = 0 ;
	int movY= 0;
	int esIo = 0;
	int tiempo_tarea = 0;
	int moveBound = 0;

	parsear_tarea(tarea,&movX,&movY,&esIo,&tiempo_tarea);


	actualizar_estado(socketRam,tripulante,READY);


	char* claveNueva = string_new();
	tripulante->instrucciones_ejecutadas = 0;
	//int tareas_pedidas = 0;
	int rafaga = 0;

	string_append(&claveNueva,tarea);
//	sem_wait(&tripulante->emergencia);
	sem_wait(&tripulante->exec);
	actualizar_estado(socketRam,tripulante,EXEC);
	while(strcmp(tarea,"FIN")!=0){

		enviar_evento_bitacora(socketMongo,tripulante->id,tarea);




		log_info(logger,"T%d - P%d : COMIENZA A EJECUTAR: %s", tripulante->id,tripulante->patota_id, tarea);
		
		moveBound = abs(movX-tripulante->ubi_x) +abs(movY-tripulante->ubi_y);
		while(tripulante->instrucciones_ejecutadas<moveBound+tiempo_tarea){

			sem_wait(&detenerReaunudarEjecucion);
			sem_post(&detenerReaunudarEjecucion);

			tripulante->instrucciones_ejecutadas++;

			sleep(CICLO_CPU);

			//La 'tarea 3' es de entrada salida
			if(esIo && tripulante->instrucciones_ejecutadas>moveBound){

				log_debug(logger,"T%d - P%d    ******   IO BOUND    *****", tripulante->id,tripulante->patota_id);
				tripulante->estado = 'B';
				tripulante->block_io_rafaga = tiempo_tarea;
				pthread_mutex_lock(&mutex_cola_ejecutados);
				queue_push(cola_ejecutados,tripulante);
				pthread_mutex_unlock(&mutex_cola_ejecutados);
				sem_post(&colaEjecutados);
				sem_post(&exec);

				//ACA INVERTIDO PARA QUE ME DIGA EFECTIVAMENTE CUANDO ESTA EN BLOCK
				actualizar_estado(socketRam,tripulante,BLOCK);


				sem_wait(&tripulante->bloq);

				enviar_tarea_a_ejecutar(socketMongo, tripulante->id, tarea);

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



			if(/*!esIo &&*/ tripulante->instrucciones_ejecutadas<=moveBound){
				log_info(logger,"T%d - P%d  															++++++++   	T%d - P%d :	CPU BOUND [MOVE]   +++++++", tripulante->id,tripulante->patota_id,tripulante->id,tripulante->patota_id);

				if( firstMove == 0){
					firstMove = 1;

					moveRight = movX -tripulante->ubi_x;;
					moveUp =  movY -tripulante->ubi_y;

				}


				if(moveRight!=0){
					if(movX>tripulante->ubi_x){
						tripulante->ubi_x++;
						moveRight--;
					}else if(movX<tripulante->ubi_x ){
						tripulante->ubi_x--;
						moveRight++;
					}
				}else if(moveUp!=0){
					 if ((movY>tripulante->ubi_y)  ){
						tripulante->ubi_y++;
						moveUp--;
					}else if (movY<tripulante->ubi_y  ){
						tripulante->ubi_y--;
						moveUp++;
					}

				}
				actualizar_ubicacion(socketRam,tripulante);
				char * evento_ubicacion = string_new();
				string_append_with_format(&evento_ubicacion,"Se movio a x: %d  y: %d",tripulante->ubi_x,tripulante->ubi_y);
				log_info(logger,"T%d - P%d: %s", tripulante->id,tripulante->patota_id, evento_ubicacion);
				enviar_evento_bitacora(socketMongo,tripulante->id,evento_ubicacion);
				free(evento_ubicacion);
			}else if(!esIo && tripulante->instrucciones_ejecutadas>moveBound){
				log_info(logger,"T%d - P%d  															++++++++   	T%d - P%d :	CPU BOUND  [TASK]   +++++++", tripulante->id,tripulante->patota_id,tripulante->id,tripulante->patota_id);
			}




			log_trace(logger,"T%d - P%d : CICLO TERMINADO", tripulante->id,tripulante->patota_id);

			rafaga++;

		}
		//Fin tarea
			tarea = pedir_tarea(socketRam, tripulante);
			log_trace(logger,"Tarea Recibida %s",tarea);
			if(strcmp(tarea,"FIN\0")!=0){
				parsear_tarea(tarea,&movX,&movY,&esIo,&tiempo_tarea);
				tripulante->instrucciones_ejecutadas = 0;
				tripulante->cantidad_tareas--;
				moveUp = 0;
				moveRight = 0;
				firstMove = 0;
			}
	
	}
	log_trace(logger,"T%d - P%d : FIN TAREAS", tripulante->id,tripulante->patota_id);
	tripulante->estado = 'F';
	pthread_mutex_lock(&mutex_cola_ejecutados);
	queue_push(cola_ejecutados,tripulante);
	pthread_mutex_unlock(&mutex_cola_ejecutados);
	sem_post(&colaEjecutados);
	sem_post(&exec);
	actualizar_estado(socketRam,tripulante,FIN);
	sendDeNotificacion(socketRam,FIN_TAREAS);
	sendDeNotificacion(socketRam,tripulante->id);
	sendDeNotificacion(socketRam,tripulante->patota_id);
	sendDeNotificacion(socketRam,tripulante->direccionLogica);
	recvDeNotificacion(socketRam);
	//liberar_conexion(socketRam);
	//liberar_conexion(socketMongo);
	free(claveNueva);

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

	uint32_t tarea_ejecutada = recvDeNotificacion(socketMongo);

	if(tarea_ejecutada==TAREA_EJECUTADA){
		log_debug(logger,"laborTripulante(): Tarea ejecutada correctamente");
	}else{
		log_trace(logger,"laborTripulante(): Ocurrio algun problema al ejecutar la tarea");
	}
}

void enviar_evento_bitacora(int socketMongo, int id, char* claveNueva) {
	int largoClave = string_length(claveNueva);
	int tamanio = 0;
	//En el buffer mando clave y luego valor
	void* buffer = malloc(string_length(claveNueva) + sizeof(uint32_t));
	memcpy(buffer + tamanio, &largoClave, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);
	memcpy(buffer + tamanio, claveNueva, string_length(claveNueva));
	tamanio += largoClave;
	sendRemasterizado(socketMongo, 9, tamanio, (void*) buffer);
	sendDeNotificacion(socketMongo, (uint32_t) id);
	free(buffer); //Malloc linea 253
}



