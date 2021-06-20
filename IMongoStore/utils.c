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
	conf_PATH_BITACORA = config_get_string_value (config, "PATH_BITACORA");

}

t_config* leer_config() {
	t_config *config;
	if((config = config_create("IMongoStore.config"))==NULL) {
		perror("No se pudo leer de la config. Revise. \n");
		exit(-1);
	}
	return config;
}

////////////////////////////////////////////FUNCIONES DE TAREAS/////////////////////////////////////
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

	if (fd_oxigeno == NULL) {
		perror("Hubo un error al abrir el arhivo");
		exit(-1);
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

	if (fd_comida == NULL) {
		perror("Hubo un error al abrir el arhivo");
		exit(-1);
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
	if (fd_basura == NULL) {
		perror("Hubo un error al abrir el arhivo");
		exit(-1);
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
	if (fd_oxigeno == NULL) {
		perror("Hubo un error al abrir el arhivo");
		exit(-1);
	}
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
	if (fd_oxigeno == NULL) {
		perror("Hubo un error al abrir el arhivo");
		exit(-1);
	}
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
	if (fd_comida == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
	}
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
	if (fd_comida == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
	}
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
	if (fd_basura == NULL) {
		perror("Hubo un error al abrir el arhivo");
		exit(-1);
	}
	fclose (fd_basura);


	//unlock del mutex para el manejo del archivo
	pthread_mutex_unlock(&mut_ARCHIVO_BASURA);
}

char* generarIdArchivo(uint32_t idTripulante) {
	char* archivoTripulante = string_new();
	string_append(&archivoTripulante, "Tripulante");
	string_append(&archivoTripulante, string_itoa(idTripulante));
	string_append(&archivoTripulante, ".ims");
	return archivoTripulante;
}

char* generarPath(char* archivoTripulante) {
	//asigno el path
	char* path_bitacora = string_new();
	string_append(&path_bitacora, conf_PUNTO_MONTAJE);
	string_append(&path_bitacora, conf_PATH_BITACORA);
	string_append(&path_bitacora, archivoTripulante);
	return path_bitacora;
}

////////////////////////////////////////////FUNCIONES DE TAREAS///////////////////////////////////////


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

/* Bitacora */

////////////////////////////////////////////FUNCIONES DE BITACORA/////////////////////////////////////
void escribirBitacora(char* tarea, uint32_t idTripulante) {


	FILE* archivoBitacora;
	char* archivoTripulante = generarIdArchivo(idTripulante);
	char* pathBitacoraTripulante = generarPath(archivoTripulante);

	archivoBitacora = fopen(pathBitacoraTripulante,"a+");
	if (archivoBitacora == NULL) {
		perror("Hubo un error al abrir el arhivo");
		exit(-1);
	}
//	tarea = strcat(tarea,"\n");
	fprintf(archivoBitacora, "%s\n",tarea);

	//cierra el archivo
	txt_close_file(archivoBitacora);


}
////////////////////////////////////////////FUNCIONES DE BITACORA/////////////////////////////////////

////////////////////////////////////////////FUNCIONES DE INICIO/  ////////////////////////////////////

//Verifica si un archikvo existe o no
//case 0: printf ("El archivo existe\n"); break;
//case -1: printf ("El archivo no existe\n"); break;
//case -2: printf ("Ocurrió un error al abrir. %d (%s)\n", errno, strerror(errno)); break;
int existsArchivo(char *fname)
{
  int fd=open(fname, O_RDONLY);
  if (fd<0)         /* error */
    return (errno==ENOENT)?-1:-2;
  /* Si no hemos salido ya, cerramos */
  close(fd);
  return 0;
}



//ANALIZA SI EXISTE O NO LA ESTRUCTURA DE ARCHIVOS
//SI NO EXISTE LA CREA
//retorna 0 OK / -1 Error
short iniciaEstructuraDeArchivos()
{
	//definiciones de variables
	int existeBlocks = 0;
	int existeSuperBloque = 0;
	int existeArchivoOxigeno = 0;
	int existeArchivoOxigenoMetadata = 0;
	int existeArchivoComida = 0;
	int existeArchivoComidaMetadata = 0;
	int existeBasura = 0;
	int existeBasuraMetadata = 0;

	int cantidadArchivos = 0;

	FILE* fd_auxiliar;

	char* path_blocks = string_new();
	char* path_superbloque = string_new();
	char* path_oxigeno = string_new();
	char* path_oxigeno_metadata = string_new();
	char* path_comida = string_new();
	char* path_comida_metadata = string_new();
	char* path_basura = string_new();
	char* path_basura_metadata = string_new();

	//Asignacion de rutas
	string_append(&path_blocks,conf_PUNTO_MONTAJE);
	string_append(&path_blocks,"Blocks");

	string_append(&path_superbloque,conf_PUNTO_MONTAJE);
	string_append(&path_superbloque,"SuperBloque");

	string_append(&path_oxigeno,conf_PUNTO_MONTAJE);
	string_append(&path_oxigeno,conf_ARCHIVO_OXIGENO_NOMBRE);

	string_append(&path_oxigeno,conf_PUNTO_MONTAJE);
	string_append(&path_oxigeno,conf_ARCHIVO_OXIGENO_NOMBRE);
	string_append(&path_oxigeno_metadata,conf_PUNTO_MONTAJE);
	string_append(&path_oxigeno_metadata,"Files/");
	string_append(&path_oxigeno_metadata,conf_ARCHIVO_OXIGENO_NOMBRE);

	string_append(&path_comida,conf_PUNTO_MONTAJE);
	string_append(&path_comida,conf_ARCHIVO_COMIDA_NOMBRE);
	string_append(&path_comida_metadata,conf_PUNTO_MONTAJE);
	string_append(&path_comida_metadata,"Files/");
	string_append(&path_comida_metadata,conf_ARCHIVO_COMIDA_NOMBRE);

	string_append(&path_basura,conf_PUNTO_MONTAJE);
	string_append(&path_basura,conf_ARCHIVO_BASURA_NOMBRE);
	string_append(&path_basura_metadata,conf_PUNTO_MONTAJE);
	string_append(&path_basura_metadata,"Files/");
	string_append(&path_basura_metadata,conf_ARCHIVO_BASURA_NOMBRE);

	//Verificacion de existencia
	existeBlocks = existsArchivo(path_blocks);
	existeSuperBloque = existsArchivo(path_superbloque);
	existeArchivoOxigeno = existsArchivo(path_oxigeno);
	existeArchivoOxigenoMetadata = existsArchivo(path_oxigeno_metadata);
	existeArchivoComida = existsArchivo(path_comida);
	existeArchivoComidaMetadata = existsArchivo(path_comida_metadata);
	existeBasura = existsArchivo(path_basura);
	existeBasuraMetadata = existsArchivo(path_basura_metadata);

	cantidadArchivos = existeBlocks + existeSuperBloque + 	existeArchivoOxigeno +
			existeArchivoOxigenoMetadata + existeArchivoComida + existeArchivoComidaMetadata +
			existeBasura + existeBasuraMetadata;

	//Crea la estructura en caso de corresponder
	if(cantidadArchivos != 0)
	{
		pthread_mutex_lock(&mut_ARCHIVO_OXIGENO);
		pthread_mutex_lock(&mut_ARCHIVO_COMIDA);
		pthread_mutex_lock(&mut_ARCHIVO_BASURA);
		pthread_mutex_lock(&mut_ARCHIVO_OXIGENO_METADATA);
		pthread_mutex_lock(&mut_ARCHIVO_COMIDA_METADATA);
		pthread_mutex_lock(&mut_ARCHIVO_BASURA_METADATA);
		pthread_mutex_lock(&mut_ARCHIVO_BLOCKS);
		pthread_mutex_lock(&mut_ARCHIVO_SUPERBLOQUE);

		fd_auxiliar = fopen(path_oxigeno, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_oxigeno_metadata, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_comida, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_comida_metadata, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_basura, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_basura_metadata, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_blocks, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_superbloque, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fclose (fd_auxiliar);

		//TODO Crear la carpeta de bitacoras de cero

		pthread_mutex_unlock(&mut_ARCHIVO_OXIGENO);
		pthread_mutex_unlock(&mut_ARCHIVO_COMIDA);
		pthread_mutex_unlock(&mut_ARCHIVO_BASURA);
		pthread_mutex_unlock(&mut_ARCHIVO_OXIGENO_METADATA);
		pthread_mutex_unlock(&mut_ARCHIVO_COMIDA_METADATA);
		pthread_mutex_unlock(&mut_ARCHIVO_BASURA_METADATA);
		pthread_mutex_unlock(&mut_ARCHIVO_BLOCKS);
		pthread_mutex_unlock(&mut_ARCHIVO_SUPERBLOQUE);
	}

	return 0;
}



////////////////////////////////////////////FUNCIONES DE INICIO/  ////////////////////////////////////
