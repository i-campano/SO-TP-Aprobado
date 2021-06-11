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

	//realizarHandshake(socketServerMiRam, DISCORDIADOR, MIRAM);

	socketServerIMongoStore = conectarAServer(valorip, 5003);

	//log_info(logger, "Planificador se conecto a MIRAM");

	//realizarHandshake(socketServerIMongoStore, DISCORDIADOR, IMONGOSTORE);

	//log_info(logger, "Planificador se conecto a IMONGOSTORE");

	//atenderIMongoStore();

	planificar();

	while(1){}

	return terminar_programa(logger,config,NULL);
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
	pthread_t hiloTripulante = malloc(sizeof(pthread_t));
	pthread_create(&hiloTripulante , &attr2,(void*) labor_tripulante_new,(void*) id_tripulante);

	infoHilos * datosHilo = (infoHilos*) malloc(sizeof(infoHilos));
	datosHilo->socket = socket;
	datosHilo->hiloAtendedor = hiloTripulante;

	pthread_mutex_lock(&mutexHilos);
	list_add(hilosParaConexiones, datosHilo);
	pthread_mutex_unlock(&mutexHilos);
}

void leer_consola() {
	log_info(logger,"INGRESE UN COMANDO: ");
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
		else if(strncmp(leido, "MONGO_DETENER", 5) == 0){
			log_info(logger,"actualizaciones mongo detenido!!!: ");
			sem_wait(&activar_actualizaciones_mongo);
		}
		else if(strncmp(leido, "REANUDAR", 1) == 0){
			log_info(logger,"PLANIFICACION REANUDADA !!!: ");
			sem_post(&sistemaEnEjecucion);
		}
		else if(strncmp(leido, "XMOSTRAR_NEW", 5) == 0){
			log_info(logger,"TRIPULANTES EN NEW!!!: ");
			mostrar_tripulantes_new();
		}
		else if(strncmp(leido, "INICIAR", 1) == 0){
			log_info(logger,"PLANIFICACION INICIADA !!!: ");
			sem_post(&iniciar_cola_ready);
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

void crear_patota(){

	char * leido = readline("INGRESAR TAREAS>");

	char * tareas = string_new();
	string_append(&tareas,leido);
	free(leido);
	int longitud_tareas = string_length(tareas);

	leido = readline("CANTIDAD TRIPULANTES>");
	uint32_t cantidad_tripulantes = (uint32_t)atoi(leido);
	free(leido);

	leido = readline("ID PATOTA>");
	uint32_t patotaId = (uint32_t)atoi(leido);
	free(leido);


	char * posiciones = string_new();
	leido = readline("INGRESAR POSICIONES>");
	string_append(&posiciones,leido);

	free(leido);
	//string_append(&posiciones,"#12-3|4&#16-5|6");
	int longitud_posiciones = string_length(posiciones);

	char * claveGet = string_new();
	string_append(&claveGet,tareas);
	string_append(&claveGet,posiciones);

	int tamanioGet = 0;



	void* buffer_patota = crear_buffer_patota(longitud_tareas,
			longitud_posiciones, patotaId, cantidad_tripulantes,
			&tamanioGet, tareas, posiciones);

	sendRemasterizado(socketServerMiRam, CREAR_PATOTA,tamanioGet,buffer_patota);

	recvDeNotificacion(socketServerMiRam);
	log_info(logger,"PATOTA CREADA OK");

	for(int i = 0 ; i<cantidad_tripulantes; i++){
		tripulantes_creados++;
		int * id = malloc(sizeof(int));
		*id = tripulantes_creados;
		log_info(logger,"Creando tripulante: %d de la patota id: %d",*id,patotaId);
		crearHiloTripulante(id);
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
	pthread_mutex_unlock(&planificacion_mutex_new);

	log_info(logger,"AGREGUE A LA COLA DE NEW");



	int socketRam = conectarAServer("127.0.0.1", 5002);
	log_info(logger,"tripulante: %d  se conecto con miram...", id);

	sendDeNotificacion(socketRam,CREAR_TRIPULANTE);


	int creado = recvDeNotificacion(socketRam);

	if(creado==TRIPULANTE_CREADO){

		log_info(logger,"TRIPULANTE CREADO, id: %d", id);
	}
	sem_wait(&tripulante->ready);
	//while(tengaTareas)


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

		sem_wait(&exec);
		log_info(logger,"Enviar tarea a IMONGO STORE %s", tarea);

		int socketMongo = conectarAServer("127.0.0.1", 5003);


		char* claveNueva = string_new();

		string_append(&claveNueva,tarea);

		//enviar_tarea_a_ejecutar(socketMongo, id, claveNueva);


		//recvDeNotificacion(socketMongo);
		//log_info(logger,"TAREA EJECUTADA CORRECTAMENTE");

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
