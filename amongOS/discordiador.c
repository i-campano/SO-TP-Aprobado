#include "discordiador.h"

unsigned int contador_hilos = 0;

int main(void) {
	iniciar_logger();
	//Inicia las colas de planificacion
	iniciarEstructurasAdministrativasPlanificador();

	t_config* config = leer_config();

	char* valor = config_get_string_value(config, "CLAVE");

	char* ip = config_get_string_value(config, "IP");

	char * valorip = "127.0.0.1";

	iniciarHiloConsola();

	socketServerMiRam = conectarAServer(valorip, 5002);

	realizarHandshake(socketServerMiRam, DISCORDIADOR, MIRAM);

	socketServerIMongoStore = conectarAServer(valorip, 5003);

	log_info(logger, "Planificador se conecto a MIRAM");

	realizarHandshake(socketServerIMongoStore, DISCORDIADOR, IMONGOSTORE);

	log_info(logger, "Planificador se conecto a IMONGOSTORE");

	//atenderLaRam();

	//atenderIMongoStore();


	planificar();

	while(1){}

	return terminar_programa(logger,config,NULL);
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

void crearHiloTripulante(int * id_tripulante){
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);
	pthread_attr_setdetachstate(&attr2, PTHREAD_CREATE_DETACHED);
	log_info(logger,"id desde crearHiloTripulante: %d",*id_tripulante);
	pthread_t hiloTripulante = malloc(sizeof(pthread_t));
	pthread_create(&hiloTripulante , &attr2,(void*) labor_tripulante_new,(void*) id_tripulante);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = socket;
	datosHilo->hiloAtendedor = hiloTripulante;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}

void *labor_tripulante_new(void * id_tripulante){
	//¿ estructura estatica dentro del hilo? --- pensar

	int id = *(int*)id_tripulante;

	int socketRam = conectarAServer("127.0.0.1", 5002);
	log_info(logger,"tripulante: %d  se conecto con miram...", id);


	sendDeNotificacion(socketRam, PEDIR_TAREA);

	sendDeNotificacion(socketRam,(uint32_t)id);
	log_info(logger,"tripulante: %d pidio tareas a miram...", id);

	uint32_t OPERACION = recvDeNotificacion(socketRam);

	log_info(logger,"OPERACION %d",OPERACION);
	char * tarea = string_new();
	if(OPERACION==ENVIAR_TAREA){
		tarea = recibirString(socketRam);
		log_info(logger,"tripulante: %d recibio tarea: %s de miram...", id,tarea);

	}
	log_info(logger,"Enviar tarea a IMONGO STORE %s", tarea);

	int socketMongo = conectarAServer("127.0.0.1", 5003);

	//sendDeNotificacion(socketMongo, EJECUTAR_TAREA);


	char* claveNueva = "CREAR OXIGENOOOO";
	int largoClave = string_length(claveNueva);
	int tamanio = 0;
	//En el buffer mando clave y luego valor
	void* buffer = malloc(string_length(claveNueva) + sizeof(uint32_t));
	memcpy(buffer + tamanio, &largoClave, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);
	memcpy(buffer + tamanio, claveNueva, string_length(claveNueva));
	tamanio += largoClave;
	sendRemasterizado(socketMongo, EJECUTAR_TAREA, tamanio, (void*) buffer);
	sendDeNotificacion(socketMongo,(uint32_t)id);


	//sendRemasterizado(socketServerIMongoStore, HANDSHAKE_TRIPULANTE);
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

void* crear_buffer_patota(int longitud_tareas, int longitud_posiciones, uint32_t patotaId, uint32_t cantidad_tripulantes, int* tamanioGet, char* tareas, char* posiciones) {
	void* buffer_patota = malloc(longitud_tareas + longitud_posiciones + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(int) + sizeof(int));

	memcpy(buffer_patota + *tamanioGet, &longitud_tareas, sizeof(int));
	*tamanioGet += sizeof(int);

	memcpy(buffer_patota + *tamanioGet, tareas, longitud_tareas);
	*tamanioGet += longitud_tareas;

	memcpy(buffer_patota + *tamanioGet, &longitud_posiciones, sizeof(int));
	*tamanioGet += sizeof(int);

	memcpy(buffer_patota + *tamanioGet, posiciones, longitud_posiciones);
	*tamanioGet += longitud_posiciones;

	memcpy(buffer_patota + *tamanioGet, &patotaId, sizeof(uint32_t));
	*tamanioGet += sizeof(uint32_t);

	memcpy(buffer_patota + *tamanioGet, &cantidad_tripulantes,
			sizeof(uint32_t));
	*tamanioGet += sizeof(uint32_t);
	return buffer_patota;
}

void leer_consola() {
	tripulantes_creados = 0;
	char* leido = readline(">");
	while(strncmp(leido, "", 1) != 0) {
		log_info(logger, leido);

		if(strncmp(leido, "DETENER", 1) == 0){
			log_info(logger,"PLANIFICACION DETENIDA !!!: ");
			sem_wait(&sistemaEnEjecucion);
		}
		else if(strncmp(leido, "ACTUALIZACIONES_MONGO", 1) == 0){
			log_info(logger,"actualizaciones mongo activado !!!: ");
			sem_post(&activar_actualizaciones_mongo);
		}
		else if(strncmp(leido, "MONGO_DETENER", 1) == 0){
			log_info(logger,"actualizaciones mongo detenido!!!: ");
			sem_wait(&activar_actualizaciones_mongo);
		}
		else if(strncmp(leido, "REANUDAR", 1) == 0){
			log_info(logger,"PLANIFICACION REANUDADA !!!: ");
			sem_post(&sistemaEnEjecucion);
		}
		else if(strncmp(leido, "INICIAR", 1) == 0){
			log_info(logger,"PLANIFICACION INICIADA !!!: ");
			sem_post(&iniciar_planificacion);
		}else if(strncmp(leido, "CREAR_PATOTA", 1) == 0){
			log_info(logger,"CARGAR DATOS PATOTA: ");

			//ID PATOTA (UINT) | CANTIDAD TRIPULANTE (UINT) | LONGITUD ->|STRING (IDS TRIPULANTES)|LONGITUD -> |STRING(X|Y-X|Y) | LONGITUD->|STRING(TAREAS)

			//TODO: CREAR UNA FUNCION QUE CREE UNA PATOTA

			//patota patota = crear_patota();
			leido = readline("INGRESAR TAREAS>");
			uint32_t patotaId = 10;
			uint32_t cantidad_tripulantes = 15;

			char * tareas = string_new();
			string_append(&tareas,leido);
			free(leido);
			int longitud_tareas = string_length(tareas);

			char * posiciones = string_new();
			leido = readline("INGRESAR POSICIONES>");
			string_append(&posiciones,leido);
			//string_append(&posiciones,"#12-3|4&#16-5|6");
			int longitud_posiciones = string_length(posiciones);

			char * claveGet = string_new();
			string_append(&claveGet,tareas);
			string_append(&claveGet,posiciones);

			int tamanioGet = 0;



			void* buffer_patota = crear_buffer_patota(longitud_tareas,
					longitud_posiciones, patotaId, cantidad_tripulantes,
					&tamanioGet, tareas, posiciones);

			queue_push(planificacion_cola_new,&patotaId);

			for(int i = 0 ; i<cantidad_tripulantes; i++){
				tripulantes_creados++;
				//el proposito:
					// tener los tripulantes para ponerlos en new
					// poder hacer los handshakes de cada tripulante para que en los otros modulos se conserve el socket de estos
					// guardar los hilos en el discordiador
				int * id = malloc(sizeof(int));
				*id = tripulantes_creados;
				crearHiloTripulante(id);
			}

			//sendRemasterizado(socketServerMiRam, CREAR_PATOTA,tamanioGet,buffer_patota);

			free(leido);
		}else{
			log_info(logger,"COMANDO INVALIDO");
		}

		leido = readline(">");

	}

	free(leido);
}

//TODO : FIX
patota crear_pcb(){
	//revisar referencias... seguro va un malloc o algo asi aca..
	patota patota;
//	patota.cantidad_tripulantes = cant_trip;
	//patota.patota_id = patota_id;

	uint32_t patotaId = 10;
	uint32_t cantidad_tripulantes = 2;

	patota.patota_id = patotaId;
	patota.cantidad_tripulantes = cantidad_tripulantes;

	patota.tareas = string_new();
	string_append(&patota.tareas,"oxigenoo");
	int longitud_tareas = string_length(patota.tareas);

	patota.posiciones = string_new();
	string_append(&patota.posiciones,"#12-3|4&#16-5|6");
	int longitud_posiciones = string_length(patota.posiciones);
	patota.longitud_posiciones = longitud_posiciones;

	char * claveGet = string_new();
	string_append(&claveGet,patota.tareas);
	string_append(&claveGet,patota.posiciones);



	return patota;

}


void pedir_tareas(){

}


void planificar_tripulantes(){
	sem_wait(&iniciar_planificacion);

	hilo_cola_new();
	hilo_cola_ready();
}

void iniciarEstructurasAdministrativasPlanificador(){
	log_info(logger, "GENERANDO ESTRUCTURAS ADMINISTRATIVAS!");

	sem_init(&iniciar_planificacion, 0, 0);
	sem_init(&iniciar_cola_ready, 0, 0);
	sem_init(&sistemaEnEjecucion, 0, 1);
	sem_init(&activar_actualizaciones_mongo, 0, 0);

	pthread_mutex_init(&planificacion_mutex_new,NULL);
	pthread_mutex_init(&planificacion_mutex_ready,NULL);

	pthread_mutex_init(&comuni,NULL);



	planificacion_cola_new = queue_create();
	planificacion_cola_ready = queue_create();

	hilosParaConexiones = list_create();

	pthread_mutex_init(&mutexHilos,NULL);
}

int realizar_operacion(char* mensaje,int conexion_mi_ram,int conexion_file_system,int conexion_tripulante) {
	int codigo_operacion;

	if(mensaje == NULL) {
		return -1;
	}
	codigo_operacion = reconocer_op_code(mensaje);
	printf("Cod OP %d \n",codigo_operacion);
	switch (codigo_operacion) {
		case INICIAR_PLANIFICACION: {
			//Accion iniciar
			break;
		}
		case PAUSAR_PLANIFICACION: {
			//Realizar Tarea acorde a INICIAR
			break;
		}
		case REANUDAR_PLANIFICACION: {
			break;
		}
		case LISTAR_TRIPULANTES: {
			break;
		}
		case EXPULSAR_TRIPULANTE: {
			break;
		}
		case BITACORA_TRIPULANTE :{
			break;
		}
		case INICIAR_PATOTA :{
			//inicializar_patota(mensaje);
			break;
		}
		default: {
			return -1;
		}
	}
	return 0;

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


void leer_consola2(t_log* logger,int conexion_ram,int conexion_fs,int conexion_trip) {
	char* leido = readline(">");
	realizar_operacion(leido,conexion_ram,conexion_fs,conexion_trip);
	while(strncmp(leido, "", 1) != 0) {
		log_info(logger, leido);
		free(leido);
		leido = readline(">");
		realizar_operacion(leido,conexion_ram,conexion_fs,conexion_trip);
	}

	free(leido);
}

//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------SIN USO - REVISAR----------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------


void *labor_tripulante (void* tripulante) {

//	tripulante_t* p_aux_trip;
//	p_aux_trip = (tripulante_t *) tripulante;

//	while (!(p_aux_trip->fin_tareas)) {
//
//		switch (p_aux_trip->estado){
//			case NEW: {
//				if (p_aux_trip->tarea != NULL){
//					//Si ya se mi tarea no la pido otra vez
//					break;
//				}
//				//Pedir tarea a mi Ram
//				//El planificador vera si estoy ready o no
//				break;
//			}
//			case READY: {
//				//Literalmente no hago nada en ready quizas informar donde estoy por algun LOG no se
//				break;
//			}
//			case EXEC: {
//				p_aux_trip->trabajando = TRUE;
//
//				//Me pongo a laburar Conectarse al MONGO y realizar los cambios en las tareas INFORMO A MI RAM
//				//Moverme a X Y posicion e informar a MI_RAM y Mongo que hice
//
//
//				//Pido siguiente tarea a MIRAM y lo guardo en *ẗarea
//				if (p_aux_trip->tarea == NULL){
//					//Termine tareas porque no hay mas
//					p_aux_trip->fin_tareas = TRUE;
//					p_aux_trip->trabajando = FALSE; //Porque ya termine
//				}
//				p_aux_trip->trabajando = FALSE; //Porque ya termine
//				//Se puede buguear!! ojo si el planificador es mas lento, muy poco posible mirando el codigo
//				break;
//			}
//			case FIN: {
//				//No se si informar aca en los logs o afuera en el planif aca se piensa
//				return NULL; //LLEGUE AL FIN TERMINO HILO
//			}
//		}
//	}

	return NULL;
}
