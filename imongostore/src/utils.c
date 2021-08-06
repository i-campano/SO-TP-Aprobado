/*
 * conexiones.c
 */

#include"utils.h"


void iniciar_configuracion(){
	exitSincro = 0;
	lista_hilos = list_create();
	config = leer_config();
	conf_LOG_LEVEL = config_get_string_value(config, "LOG_LEVEL");
	t_log_level log_level = log_level_from_string(conf_LOG_LEVEL);
	logger = log_create("IMongoStore.log", "IMongoStore", 1,log_level);


	conf_PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
	conf_PUERTO_IMONGO = config_get_int_value (config, "PUERTO");
	conf_TIEMPO_SINCRONIZACION = config_get_int_value (config, "TIEMPO_SINCRONIZACION");
	//conf_TIEMPO_SICRONIZACION = config_get_int_value (config, "TIEMPO_SICRONIZACION");
	conf_POSICIONES_SABOTAJE = config_get_array_value(config, "POSICIONES_SABOTAJE");
	conf_TIEMPO_SABOTAJE = config_get_int_value(config, "TIEMPO_SABOTAJE");
	//conf_PUERTO_DISCORDIADOR = config_get_int_value (config, "PUERTO_DISCORDIADOR");
	//conf_IP_DISCORDIADOR = config_get_string_value (config, "IP_DISCORDIADOR");
	conf_ARCHIVO_OXIGENO_NOMBRE = config_get_string_value (config, "ARCHIVO_OXIGENO_NOMBRE");
	conf_ARCHIVO_COMIDA_NOMBRE = config_get_string_value (config, "ARCHIVO_COMIDA_NOMBRE");
	conf_ARCHIVO_BASURA_NOMBRE = config_get_string_value (config, "ARCHIVO_BASURA_NOMBRE");
	conf_PATH_BITACORA = config_get_string_value (config, "PATH_BITACORA");
	conf_PATH_FILES = config_get_string_value (config, "PATH_FILES");


	conf_BYTES_BLOQUE = config_get_string_value (config, "BYTES_BLOQUE");
	conf_CANTIDAD_BLOQUES = config_get_string_value (config, "CANTIDAD_BLOQUES");

	sabotajes_realizados = 0;

	archivos_bitacora = list_create();
	pthread_mutex_init(&mutex_archivos_bitacora,NULL);
}

t_config* leer_config() {
	t_config *config;
	if((config = config_create("IMongoStore.config"))==NULL) {
		perror("No se pudo leer de la config. Revise. \n");
		exit(-1);
	}
	return config;
}


void init_server(){


	log_info(logger, "FS_SERVER OK");
}

void manejadorDeHilos(){
	int socketCliente;

	// Funcion principal
	while((socketCliente = aceptarConexionDeCliente(fs_server))) { 	// hago el accept
		pthread_t * thread_id = malloc(sizeof(pthread_t));
    	pthread_attr_t attr;
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    	int * pcclient = malloc(sizeof(int)); // TODO : ESTO SI LO SACABA DEJABA DE FUNCIONAR. PROBAR
    	*pcclient = socketCliente;
		//Creo hilo atendedor
		pthread_create( thread_id , &attr, (void*) atenderNotificacion , (void*) pcclient);
		list_add(lista_hilos,thread_id);

	}


	//Chequeo que no falle el accept
}

void enviar_bitacora(int socket, char * tarea) {
	char* claveNueva = tarea;
	int largoClave = string_length(claveNueva);
	int tamanio = 0;
	//En el buffer mando clave y luego valor
	void* buffer = malloc(string_length(claveNueva) + sizeof(uint32_t));
	memcpy(buffer + tamanio, &largoClave, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);
	memcpy(buffer + tamanio, claveNueva, string_length(claveNueva));
	tamanio += largoClave;
	sendRemasterizado(socket, ENVIAR_BITACORA, tamanio, (void*) buffer);
}

void *atenderNotificacion(void * paqueteSocket){

	int socket = *(int*)paqueteSocket;

	while(1){

		log_trace(logger,"espero mas notificaciones....");
		uint32_t nroNotificacion = recvDeNotificacion(socket);

		switch(nroNotificacion){

			log_info(logger,"en el switch....");


			case DISCORDIADOR:{
				log_info(logger,"Se ha conectado el DISCORDIADOR");
				sendDeNotificacion(socket, IMONGOSTORE);
				socketDiscordiador = socket;
				break;
			}

			case EJECUTAR_TAREA:{
				char * tarea = recibirString(socket);
				uint32_t id_trip = recvDeNotificacion(socket);
				log_debug(logger,"atenderNotificacion(): Tripulante %d ejecuta tarea: %s",id_trip,tarea);
				tipoTarea(tarea,id_trip);
				sendDeNotificacion(socket,TAREA_EJECUTADA);


				break;
			}
			case LOGUEAR_BITACORA:{
				char * tarea = recibirString(socket);
				uint32_t id_trip = recvDeNotificacion(socket);
				log_debug(logger,"atenderNotificacion(): Id tripulante %d escribe en bitacora %s",id_trip,tarea);

				char * nombre_archivo = string_from_format("tripulante_%d",id_trip);
				_archivo_bitacora * archivo = iniciar_archivo_bitacora(nombre_archivo,"tarea1");
				write_archivo_bitacora(tarea,archivo);

				break;
			}
			case FSCK:{
				log_debug(logger,"atenderNotificacion(): EJECUTANDO FSCK");
				log_info(logger,"EJECUTANDO FSCK");
				fsck();
				log_info(logger,"TERMINO FSCK");
				log_debug(logger,"atenderNotificacion(): termino FSCK aviso al tripulante");
				sendDeNotificacion(socket,SABOTAJE_RESUELTO);
				break;
			}
			case PEDIR_BITACORA:{
				int num_trip = (int)recibirUint(socket);
				log_debug(logger,"atenderNotificacion(): Pedido bitacora tripulante %d", num_trip);
				char * bitacora = obtener_bitacora(num_trip);
				enviar_bitacora(socket,bitacora);
				break;
			}

			default:
				log_warning(logger, "atenderNotificacion(): La conexion recibida es erronea");
				close(socket);
				return 0;
				break;
		}
	}
	return 0;
}

void ejecutar_tarea(char * tarea,char caracter_tarea,_archivo * archivo, uint32_t id_trip){
	int cantidad = parsear_tarea(tarea);
	char * cadena = string_repeat(caracter_tarea,cantidad);
	write_archivo(cadena,archivo,id_trip);
}


void ejecutar_tarea_consumir(char * tarea,char caracter_tarea,_archivo * archivo,uint32_t id_trip){
	int cantidad = parsear_tarea(tarea);
	char * cadena = string_repeat(caracter_tarea,cantidad);
	consumir_arch(archivo,cantidad,id_trip);
}


int parsear_tarea(char* tarea,int cantidad_caracteres) {
	char** tarea_separada = string_split(tarea,";");
	char** tarea_parametro = string_split(tarea_separada[0]," ");

	if(tarea_parametro[1] == NULL) {
		cantidad_caracteres = 0;
	}else{
		cantidad_caracteres = atoi(tarea_parametro[1]);
	}
	log_debug(logger,"parsear_tarea(): cantidad de caracteres %d",cantidad_caracteres);

	free(tarea_parametro);//stringsplit
	return cantidad_caracteres;
}





void tipoTarea(char* tarea, uint32_t id_trip){

	if (strncmp("GENERAR_OXIGENO", tarea,12)==0){
		log_debug(logger,"generando OXIGENO...");
		ejecutar_tarea(tarea,'O',archivo_oxigeno,id_trip);
	}
	else if (strncmp("GENERAR_COMIDA", tarea,12)==0){
		log_debug(logger,"GENERANDO COMIDA...");
		ejecutar_tarea(tarea,'C',archivo_comida,id_trip);
	}
	else if ( strncmp("GENERAR_BASURA", tarea,12)==0) {
		log_debug(logger,"GENERANDO BASURA...");
		ejecutar_tarea(tarea,'B',archivo_basura,id_trip);
	}
	else if (strncmp("CONSUMIR_OXIGENO", tarea,12)==0) {
		log_debug(logger,"consumiendo OXIGENO...");
		ejecutar_tarea_consumir(tarea,'O',archivo_oxigeno,id_trip);
	}
	else if ( strncmp("CONSUMIR_COMIDA", tarea,12)==0) {
		log_debug(logger,"consumiendo COMIDA...");
		ejecutar_tarea_consumir(tarea,'C',archivo_comida,id_trip);
	}
	else if ( strncmp("CONSUMIR_BASURA", tarea,12)==0) {
		log_debug(logger,"consumiendo COMIDA...");
		ejecutar_tarea_consumir(tarea,'B',archivo_basura,id_trip);
	}
	else if (strncmp("DESCARTAR_BASURA", tarea,12)==0){
		log_debug(logger,"descartando BASURA...");
		descartar_basura(archivo_basura,id_trip);
	}
	else {
		log_debug(logger,"Tarea desconocida"); //TODO manejar que hacemos en caso de que la tarea no exista
	}

}
