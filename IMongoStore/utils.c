/*
 * conexiones.c
 */

#include"utils.h"



void manejadorDeHilos(){
	int socketCliente;

	// Funcion principal
	while((socketCliente = aceptarConexionDeCliente(server_fd))) { 	// hago el accept
		pthread_t * thread_id = malloc(sizeof(pthread_t));
    	pthread_attr_t attr;
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    	int * pcclient = malloc(sizeof(int));
    	*pcclient = socketCliente;
		//Creo hilo atendedor
		pthread_create( thread_id , &attr, (void*) atenderNotificacion , (void*) pcclient);
		pthread_detach(thread_id);

	}

	//Chequeo que no falle el accept
}


void *atenderNotificacion(void * paqueteSocket){

	int socket = *(int*)paqueteSocket;

	while(1){

		log_info(logger,"espero mas notificaciones....");
		uint32_t nroNotificacion = recvDeNotificacion(socket);

		switch(nroNotificacion){

			log_info(logger,"en el switch....");


			case DISCORDIADOR:{
				//Case para hacer HANDSHAKE = Chequear la conexion
				log_info(logger,"Se ha conectado el DISCORDIADOR");
				sendDeNotificacion(socket, IMONGOSTORE);
				//while(1){
					//sleep(10);
					//log_info(logger,"ENVIANDO TAREAS");
					//sendDeNotificacion(socket, ACTUALIZACION_IMONGOSTORE);
				//}
			}
				break;

			case EJECUTAR_TAREA:{
				char * tarea = recibirString(socket);
				//Case para hacer HANDSHAKE = Chequear la conexion

				uint32_t id_trip = recvDeNotificacion(socket);
				ejecutar_tarea(tarea);
				sendDeNotificacion(socket,198);
				log_info(logger,"Id tripulante %d quiere hacer la tarea: %s",id_trip,tarea);

			}
				break;
			default:
				log_warning(logger, "La conexion recibida es erronea");
				break;
		}
	}
	return 0;
}

void ejecutar_tarea(char * tarea)
{

	if(strcmp(tarea,"GENERAR_OXIGENO 12;2;3;5")==0)
	{
		log_info(logger,"EJECUTO TAREA - GENERAR OXIGENO:");
		log_info(logger,"OOOOOOOOOOO");

	}

}

void iniciar_configuracion(){
	t_config* config = leer_config();

	conf_PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
	conf_PUERTO_IMONGO = config_get_int_value (config, "PUERTO");
	//conf_TIEMPO_SICRONIZACION = config_get_int_value (config, "TIEMPO_SICRONIZACION");
	//conf_POSICIONES_SABOTAJE = config_get_string_value(config, "POSICIONES_SABOTAJE");
	//conf_PUERTO_DISCORDIADOR = config_get_int_value (config, "PUERTO_DISCORDIADOR");
	//conf_IP_DISCORDIADOR = config_get_string_value (config, "IP_DISCORDIADOR");
	conf_ARCHIVO_OXIGENO_NOMBRE = config_get_string_value (config, "ARCHIVO_OXIGENO_NOMBRE");
	conf_ARCHIVO_COMIDA_NOMBRE = config_get_string_value (config, "ARCHIVO_COMIDA_NOMBRE");
	conf_ARCHIVO_BASURA_NOMBRE = config_get_string_value (config, "ARCHIVO_BASURA_NOMBRE");

}

t_config* leer_config() {
	t_config *config;
	if((config = config_create("IMongoStore.config"))==NULL) {
		printf("No se pudo leer de la config. Revise. \n");
		exit(1);
	}
	return config;
}

////////FUNCIONES DE TAREAS/////////
void generarDatos(uint32_t cantidad, char caracter)
{
	//definiciones de variables
	char* path_oxigeno = string_new();
	char* cadenaOxigenos;
	FILE* fd_oxigeno;

	//asigno el path
	string_append(&path_oxigeno,conf_PUNTO_MONTAJE);
	string_append(&path_oxigeno,conf_ARCHIVO_OXIGENO_NOMBRE);

	//lock del mutex para el manejo del archivo
	pthread_mutex_lock(&mut_ARCHIVO_OXIGENO);

	//escribe el archivo
	log_info(logger,"EJECUTO TAREA - GENERAR OXIGENO: ABRO ARCHIVO"); //TODO agregar path y cantidad al log
	fd_oxigeno = txt_open_for_append(path_oxigeno);

	if (fd_oxigeno == NULL)
	{
		//TODO Manejo del error
	}

	//Escribe la cantidad  OXIGENO SOLICITADA
	cadenaOxigenos = string_repeat(caracter, cantidad);
	txt_write_in_file(fd_oxigeno, cadenaOxigenos);

	//cierra el archivo
	txt_close_file(fd_oxigeno);

	//ulock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_OXIGENO);
}

void consumirDatos(uint32_t cantidad, char caracter)
{
	//definiciones de variables
	char* path_oxigeno = string_new();
	char* cadenaOxigenos = string_new();
	char* cadenaOxigenosAux = string_new();
	FILE* fd_oxigeno;

	//asigno el path
	string_append(&path_oxigeno, conf_PUNTO_MONTAJE);
	string_append(&path_oxigeno, conf_ARCHIVO_OXIGENO_NOMBRE);

	//lock del mutex para el manejo del archivo
	pthread_mutex_lock(&mut_ARCHIVO_OXIGENO);

	//lee la cadena dentro del archivo
	fd_oxigeno = fopen(path_oxigeno, "r");
	fscanf(fd_oxigeno,"%s",cadenaOxigenosAux);
	//Si la cantidad a borrar es mayor de la lo que hay escrito, deja el archivo en blanco
	if (string_length(cadenaOxigenosAux)>cantidad) {
		cadenaOxigenos = string_substring(cadenaOxigenosAux,0,string_length(cadenaOxigenosAux)-cantidad);
	}
	fclose (fd_oxigeno);

	fd_oxigeno = fopen(path_oxigeno, "w");
	fputs(cadenaOxigenos,fd_oxigeno);
	fclose (fd_oxigeno);


	//unlock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_OXIGENO);
}

/* TODO: implementar
void descartarBasura();*/
////////FUNCIONES DE TAREAS/////////

