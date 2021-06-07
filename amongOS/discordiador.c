#include "discordiador.h"

//Creacion de las colas de planificacion. Son Globales en el discordiador


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

	log_info(logger, "Planificador se conecto a MIRAM");

	atenderLaRam();

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



void atender_ram(){
	uint32_t notificacion;
	while(1){

		notificacion = recibirUint(socketServerMiRam);

		switch(notificacion){
		case PATOTA_CREADA:
			log_info(logger,"PATOTA CREADA EN MI RAM");
			log_info(logger,"PASAMOS PATOTA DE NEW A READY");

			int * patota = (int *) queue_pop(planificacion_cola_new);

			queue_push(planificacion_cola_ready, patota);

			int * patota2 = (int *) queue_pop(planificacion_cola_ready);
			char * patotaString = string_itoa(*patota2);

			log_info(logger,"PATOTA EN READY: ");
			log_info(logger,patotaString);
			sem_post(&iniciar_cola_ready);
		}
	}
}

void leer_consola() {
	char* leido = readline(">");
	while(strncmp(leido, "", 1) != 0) {
		log_info(logger, leido);

		if(string_length(leido)==6){
			sem_wait(&sistemaEnEjecucion);
		}
		else if(string_length(leido)==7){
			sem_post(&sistemaEnEjecucion);
		}
		else if(string_length(leido)<5){
			sem_post(&iniciar_planificacion);
		}else{

			//ID PATOTA (UINT) | CANTIDAD TRIPULANTE (UINT) | LONGITUD ->|STRING (IDS TRIPULANTES)|LONGITUD -> |STRING(X|Y-X|Y) | LONGITUD->|STRING(TAREAS)
			uint32_t patotaId = 10;
			uint32_t cantidad_tripulantes = 2;

			char * tareas = string_new();
			string_append(&tareas,leido);
			int longitud_tareas = string_length(tareas);

			char * posiciones = string_new();
			string_append(&posiciones,"#12-3|4&#16-5|6");
			int longitud_posiciones = string_length(posiciones);

			char * claveGet = string_new();
			string_append(&claveGet,tareas);
			string_append(&claveGet,posiciones);

			//Serializo la clave a enviar.

			int tamanioGet = 0;

			void * claveBloqueadaGet = malloc(longitud_tareas + longitud_posiciones  + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(int)+ sizeof(int));

			memcpy(claveBloqueadaGet + tamanioGet, &longitud_tareas, sizeof(int));

			tamanioGet += sizeof(int);

			memcpy(claveBloqueadaGet + tamanioGet, tareas, longitud_tareas);

			tamanioGet += longitud_tareas;

			memcpy(claveBloqueadaGet + tamanioGet, &longitud_posiciones, sizeof(int));

			tamanioGet += sizeof(int);

			memcpy(claveBloqueadaGet + tamanioGet, posiciones, longitud_posiciones);

			tamanioGet += longitud_posiciones;

			memcpy(claveBloqueadaGet + tamanioGet, &patotaId, sizeof(uint32_t));

			tamanioGet += sizeof(uint32_t);

			memcpy(claveBloqueadaGet + tamanioGet, &cantidad_tripulantes, sizeof(uint32_t));

			tamanioGet += sizeof(uint32_t);

			log_info(logger,"creo patota y agrego a cola new");

			queue_push(planificacion_cola_new,&patotaId);
			log_info(logger,"se la envio a mi ram");
			sendRemasterizado(socketServerMiRam, CREAR_PATOTA,tamanioGet,claveBloqueadaGet);
			free(leido);
		}

		leido = readline(">");

	}

	free(leido);
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

	pthread_mutex_init(&planificacion_mutex_new,NULL);
	pthread_mutex_init(&planificacion_mutex_ready,NULL);


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
			enviar_mensaje("Inicio_Planificacion\0",conexion_mi_ram);
			break;
		}
		case PAUSAR_PLANIFICACION: {
			//Realizar Tarea acorde a INICIAR
			enviar_mensaje("Pausar_Planificacion",conexion_mi_ram);
			break;
		}
		case REANUDAR_PLANIFICACION: {
			enviar_mensaje("Reanudar_planificacion",conexion_mi_ram);
			break;
		}
		case LISTAR_TRIPULANTES: {
			enviar_mensaje("Listar_Trip",conexion_file_system);
			break;
		}
		case EXPULSAR_TRIPULANTE: {
			enviar_mensaje("Hay_que_rajar_a_1",conexion_mi_ram);
			break;
		}
		case BITACORA_TRIPULANTE :{
			enviar_mensaje("Quiero_la_bitacora",conexion_file_system);
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
	close(conexion[RAM]);
	close(conexion[FILE_SYSTEM]);
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
