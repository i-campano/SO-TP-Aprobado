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

			case EJECUTAR_TAREA:

				/*char * tarea = recibirString(socket);
				//Case para hacer HANDSHAKE = Chequear la conexion
				char * accionTarea = devolverDeTarea(tarea,0); "GENERAR_OXIGENO"
				uint32_t cantidad = atoi(devolverDeTarea(tarea,1)); 10
				uint32_t id_trip = recvDeNotificacion(socket);
				ejecutar_tarea(accionTarea, 1);
				sendDeNotificacion(socket,198);
				log_info(logger,"Id tripulante %d quiere hacer la tarea: %s",id_trip,tarea);*/

				break;
			default:
				log_warning(logger, "La conexion recibida es erronea");
				break;
		}
	}
	return 0;
}
//TODO, mejorar con codigos como est'a hecho arriba
void ejecutarTarea(char* tarea, uint32_t  cantidad){

	if (strcmp("GENERAR_OXIGENO", tarea)==0){
			generarOxigeno(cantidad);
	}
	else if (strcmp("GENERAR_COMIDA", tarea)==0){
		generarComida(cantidad);
	}
	else if ( strcmp("GENERAR_BASURA", tarea)==0) {
		generarBasura(cantidad);
	}
	else if (strcmp("CONSUMIR_OXIGENO", tarea)==0) {
		consumirOxigeno(cantidad);
	}
	else if ( strcmp("CONSUMIR_COMIDA", tarea)==0) {
		consumirComida(cantidad);
	}
	else if (strcmp("DESCARTAR_BASURA", tarea)==0){
		descartarBasura();
	}
	else {
			printf("Tarea desconocida"); //TODO manejar que hacemos en caso de que la tarea no exista
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
void generarOxigeno(uint32_t cantidad)
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
	cadenaOxigenos = string_repeat('O', cantidad);
	txt_write_in_file(fd_oxigeno, cadenaOxigenos);

	//cierra el archivo
	txt_close_file(fd_oxigeno);
	free(path_oxigeno);
	//ulock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_OXIGENO);
}

void generarComida(uint32_t cantidad)
{
	//definiciones de variables
	char* path_comida = string_new();
	char* cadenaComida;
	FILE* fd_comida;

	//asigno el path
	string_append(&path_comida, conf_PUNTO_MONTAJE);
	string_append(&path_comida, conf_ARCHIVO_COMIDA_NOMBRE);

	//lock del mutex para el manejo del archivo
	pthread_mutex_lock(&mut_ARCHIVO_COMIDA);

	//escribe el archivo
	log_info(logger,"EJECUTO TAREA - GENERAR COMIDA: ABRO ARCHIVO"); //TODO agregar path y cantidad al log
	fd_comida = txt_open_for_append(path_comida);

	if (fd_comida == NULL)
	{
		//TODO Manejo del error
	}

	//Escribe la cantidad  OXIGENO SOLICITADA
	cadenaComida = string_repeat('C', cantidad);
	txt_write_in_file(fd_comida, cadenaComida);

	//cierra el archivo
	txt_close_file(fd_comida);
	free(path_comida);
	//ulock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_COMIDA);
}

void generarBasura(uint32_t cantidad)
{
	//definiciones de variables
	char* path_basura = string_new();
	char* cadenaBasura;
	FILE* fd_basura;

	//asigno el path
	string_append(&path_basura ,conf_PUNTO_MONTAJE);
	string_append(&path_basura ,conf_ARCHIVO_BASURA_NOMBRE);

	//lock del mutex para el manejo del archivo
	pthread_mutex_lock(&mut_ARCHIVO_BASURA);

	//escribe el archivo
	log_info(logger,"EJECUTO TAREA - GENERAR BASURA: ABRO ARCHIVO"); //TODO agregar path y cantidad al log
	fd_basura = txt_open_for_append(path_basura);

	if (fd_basura == NULL)
	{
		//TODO Manejo del error
	}

	//Escribe la cantidad  OXIGENO SOLICITADA
	cadenaBasura = string_repeat('B', cantidad);
	txt_write_in_file(fd_basura, cadenaBasura);

	//cierra el archivo
	txt_close_file(fd_basura);

	//ulock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_BASURA);
}


void consumirOxigeno(uint32_t cantidad)
{

	//definiciones de variables
	char* path_oxigeno = string_new();
	char* cadenaOxigenos = string_new();
	char* cadenaOxigenosAux = string_new();
	FILE* fd_oxigeno;
	int caracter;
	int contador=0;

	//asigno el path
	string_append(&path_oxigeno, conf_PUNTO_MONTAJE);
	string_append(&path_oxigeno, conf_ARCHIVO_OXIGENO_NOMBRE);

	//lock del mutex para el manejo del archivo
	pthread_mutex_lock(&mut_ARCHIVO_OXIGENO);

	//lee la cadena dentro del archivo
	fd_oxigeno = fopen(path_oxigeno, "r");
	caracter = fgetc(fd_oxigeno);
	while(caracter!=EOF){
		caracter = fgetc(fd_oxigeno);
		contador++;
	}
	cadenaOxigenosAux = string_repeat('O',contador);
	printf("contador: %d", contador);
	if (contador > cantidad) {
		cadenaOxigenos = string_substring(cadenaOxigenosAux,0,string_length(cadenaOxigenosAux)-cantidad);
	}
	fclose (fd_oxigeno);

	fd_oxigeno = fopen(path_oxigeno, "w");
	fputs(cadenaOxigenos,fd_oxigeno);
	fclose (fd_oxigeno);
	log_info(logger,"EJECUTO TAREA - CONSUMIR OXIGENO ABRO ARCHIVO"); //TODO agregar path y cantidad al log


	//unlock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_OXIGENO);
}

void consumirComida(uint32_t cantidad)
{
	//definiciones de variables
	char* path_comida = string_new();
	char* cadenaComida = string_new();
	char* cadenaComidaAux = string_new();
	FILE* fd_comida;
	int caracter,contador=0;

	//asigno el path
	string_append(&path_comida, conf_PUNTO_MONTAJE);
	string_append(&path_comida, conf_ARCHIVO_COMIDA_NOMBRE);


	//lock del mutex para el manejo del archivo
	pthread_mutex_lock(&mut_ARCHIVO_COMIDA);

	//lee la cadena dentro del archivo
	fd_comida = fopen(path_comida, "r");
	caracter = fgetc(fd_comida);
	while(caracter!=EOF){
		caracter = fgetc(fd_comida);
		contador++;
	}
	cadenaComidaAux = string_repeat('C',contador);


	//Si la cantidad a borrar es mayor de la lo que hay escrito, deja el archivo en blanco
	if (string_length(cadenaComidaAux)>cantidad) {
		cadenaComida = string_substring(cadenaComidaAux,0,string_length(cadenaComidaAux)-cantidad);
	}
	fclose (fd_comida);
	fd_comida = fopen(path_comida,"w");

	//fd_comida = fopen(path_comida, "w");
	fputs(cadenaComida, fd_comida);
	fclose (fd_comida);


	//unlock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_COMIDA);
}

void descartarBasura()
{
	//definiciones de variables
	char* path_basura = string_new();
	FILE* fd_basura;

	//asigno el path
	string_append(&path_basura, conf_PUNTO_MONTAJE);
	string_append(&path_basura, conf_ARCHIVO_BASURA_NOMBRE);

	//lock del mutex para el manejo del archivo
	pthread_mutex_lock(&mut_ARCHIVO_BASURA);



	fd_basura = fopen(path_basura, "w");
	fclose (fd_basura);


	//unlock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_BASURA);
}

////////FUNCIONES DE TAREAS/////////


/*
char *devolverDeTarea(char* tarea,int dato){
	//dato = 0 devuelve el nombre de la tarea
	//dato= 1 devuelve la cantidad pasada como parametro
	int i=0;
	char laTarea[100] = "GENERAR_OXIGENO 10 4|4 5";
	char* tareas[4];
	char* token =strtok(laTarea," ");
	while(token!= NULL){
		tareas[i] = token;
		printf("\ntoken: %s",token);
		token = strtok(NULL," ");
		i++;
	}
	return tareas[dato];
}
*/

