/*
 * conexiones.c
 */

#include"utils.h"


void iniciar_configuracion(){

	config = leer_config();
	conf_LOG_LEVEL = config_get_string_value(config, "LOG_LEVEL");
	t_log_level log_level = log_level_from_string(conf_LOG_LEVEL);
	logger = log_create("IMongoStore.log", "IMongoStore", 1,log_level);


	conf_PUNTO_MONTAJE = config_get_string_value(config, "PUNTO_MONTAJE");
	conf_PUERTO_IMONGO = config_get_int_value (config, "PUERTO");
	conf_TIEMPO_SINCRONIZACION = config_get_int_value (config, "TIEMPO_SINCRONIZACION");
	//conf_TIEMPO_SICRONIZACION = config_get_int_value (config, "TIEMPO_SICRONIZACION");
	conf_POSICIONES_SABOTAJE = config_get_array_value(config, "POSICIONES_SABOTAJE");
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
	if((config = config_create("config/IMongoStore.config"))==NULL) {
		perror("No se pudo leer de la config. Revise. \n");
		exit(-1);
	}
	return config;
}


void init_server(){


	log_info(logger, "FS_SERVER OK");
}

void manejadorDeHilos(){
	log_info(logger,"1");
	int socketCliente;

	// Funcion principal
	while((socketCliente = aceptarConexionDeCliente(fs_server))) { 	// hago el accept
		log_info(logger,"2");
		pthread_t * thread_id = malloc(sizeof(pthread_t));
    	pthread_attr_t attr;
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    	int * pcclient = malloc(sizeof(int));
    	*pcclient = socketCliente;
		//Creo hilo atendedor
		pthread_create( thread_id , &attr, (void*) atenderNotificacion , (void*) pcclient);

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
				//Case para hacer HANDSHAKE = Chequear la conexion
				log_info(logger,"Se ha conectado el DISCORDIADOR");
				sendDeNotificacion(socket, IMONGOSTORE);
				socketDiscordiador = socket;
				//while(1){
					//sleep(10);
					//log_info(logger,"ENVIANDO TAREAS");
					//sendDeNotificacion(socket, ACTUALIZACION_IMONGOSTORE);
				//}
				break;
			}

			case EJECUTAR_TAREA:{
				char * tarea = recibirString(socket);
				//Case para hacer HANDSHAKE = Chequear la conexion
				uint32_t id_trip = recvDeNotificacion(socket);
				log_debug(logger,"atenderNotificacion(): Tripulante %d ejecuta tarea: %s",id_trip,tarea);
				tipoTarea(tarea);
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
				log_info(logger,"atenderNotificacion(): EJECUTO FSCK");
				fsck();
				break;
			}
			case PEDIR_BITACORA:{
				int num_trip = (int)recibirUint(socket);
				log_info(logger,"atenderNotificacion(): Pedido bitacora tripulante %d", num_trip);
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

void ejecutar_tarea(char * tarea,char caracter_tarea,_archivo * archivo){
	int cantidad = parsear_tarea(tarea);
	char * cadena = string_repeat(caracter_tarea,cantidad);
	write_archivo(cadena,archivo);
}


void ejecutar_tarea_consumir(char * tarea,char caracter_tarea,_archivo * archivo){
	int cantidad = parsear_tarea(tarea);
	char * cadena = string_repeat(caracter_tarea,cantidad);
	consumir_arch(archivo,cantidad);
}


int parsear_tarea(char* tarea,int cantidad_caracteres) {
	char** tarea_separada = string_split(tarea,";");
	char** tarea_parametro = string_split(tarea_separada[0]," ");

	if(tarea_parametro[1] == NULL) {
		cantidad_caracteres = 0;
	}else{
		cantidad_caracteres = atoi(tarea_parametro[1]);
	}
	log_info(logger,"cantidad de caracteres %d",cantidad_caracteres);
	free(tarea_parametro);//stringsplit
	return cantidad_caracteres;
}





void tipoTarea(char* tarea){

	if (strncmp("GENERAR_OXIGENO", tarea,12)==0){
		log_debug(logger,"generando OXIGENO...");
		ejecutar_tarea(tarea,'O',archivo_oxigeno);
	}
	else if (strncmp("GENERAR_COMIDA", tarea,12)==0){
		log_debug(logger,"GENERANDO COMIDA...");
		ejecutar_tarea(tarea,'C',archivo_comida);
	}
	else if ( strncmp("GENERAR_BASURA", tarea,12)==0) {
		log_debug(logger,"GENERANDO BASURA...");
		ejecutar_tarea(tarea,'B',archivo_basura);
	}
	else if (strncmp("CONSUMIR_OXIGENO", tarea,12)==0) {
		log_debug(logger,"consumiendo OXIGENO...");
		ejecutar_tarea_consumir(tarea,'O',archivo_oxigeno);
	}
	else if ( strncmp("CONSUMIR_COMIDA", tarea,12)==0) {
		log_debug(logger,"consumiendo COMIDA...");
		ejecutar_tarea_consumir(tarea,'C',archivo_comida);
	}
	else if ( strncmp("CONSUMIR_BASURA", tarea,12)==0) {
		log_debug(logger,"consumiendo COMIDA...");
		ejecutar_tarea_consumir(tarea,'B',archivo_basura);
	}
	else if (strncmp("DESCARTAR_BASURA", tarea,12)==0){
		log_debug(logger,"descartando BASURA...");
		descartar_basura(archivo_basura);
	}
	else {
		log_debug(logger,"Tarea desconocida"); //TODO manejar que hacemos en caso de que la tarea no exista
		}

}


//FUNCIONES QUE HAY QUE BORRAR------------------------------------------------------------------------------------------------------------------------------------


void loggearBitacora(char* infoALoggear, uint32_t idTripulante) {
	escribirBitacora(infoALoggear, idTripulante);
}






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

	fd_basura = fopen(path_basura, "w");
	if (fd_basura == NULL) {
		perror("Hubo un error al abrir el arhivo");
		exit(-1);
	}
	fclose (fd_basura);


	//unlock del mutex para el manejo del archivo
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
	int cantidadArchivos = 0;
	uint32_t iTamanioBloque = 0;
	uint32_t iCantidadBloques = 0;
	uint32_t iCantidadTotalDeBytesEnArchivoBlocks = 0;

	str_superblock str_FileSupeBloque;

	FILE* fd_auxiliar;

	char* path_blocks = string_new();
	char* path_superbloque = string_new();
	char* path_oxigeno_metadata = string_new();
	char* path_comida_metadata = string_new();
	char* path_basura_metadata = string_new();

	char* cadena_auxiliar = string_new();

	//Asignacion de rutas
	string_append(&path_blocks,conf_PUNTO_MONTAJE);
	string_append(&path_blocks,"Blocks");

	string_append(&path_superbloque,conf_PUNTO_MONTAJE);
	string_append(&path_superbloque,"SuperBloque");

	string_append(&path_oxigeno_metadata,conf_PUNTO_MONTAJE);
	string_append(&path_oxigeno_metadata,"Files/");
	string_append(&path_oxigeno_metadata,conf_ARCHIVO_OXIGENO_NOMBRE);

	string_append(&path_comida_metadata,conf_PUNTO_MONTAJE);
	string_append(&path_comida_metadata,"Files/");
	string_append(&path_comida_metadata,conf_ARCHIVO_COMIDA_NOMBRE);

	string_append(&path_basura_metadata,conf_PUNTO_MONTAJE);
	string_append(&path_basura_metadata,"Files/");
	string_append(&path_basura_metadata,conf_ARCHIVO_BASURA_NOMBRE);

	//Verificacion de existencia
	existeBlocks = existsArchivo(path_blocks);
	existeSuperBloque = existsArchivo(path_superbloque);

	cantidadArchivos = existeBlocks + existeSuperBloque;

	//Crea la estructura en caso de corresponder
	if(cantidadArchivos != 0)
	{


		//Asignacion de variables para manejo de bloques
		iTamanioBloque = atoi(conf_BYTES_BLOQUE);
		iCantidadBloques = atoi(conf_CANTIDAD_BLOQUES);
		iCantidadTotalDeBytesEnArchivoBlocks = iTamanioBloque * iCantidadBloques;

		//Creacion Oxigeno Metadata
		fd_auxiliar = fopen(path_oxigeno_metadata, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		cadena_auxiliar = "SIZE=0\nBLOCK_COUNT=0\nBLOCKS=[]\nCARACTER_LLENADO=O\nMD5_ARCHIVO=\n";
		txt_write_in_file(fd_auxiliar, cadena_auxiliar);
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_comida_metadata, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		cadena_auxiliar = "SIZE=0\nBLOCK_COUNT=0\nBLOCKS=[]\nCARACTER_LLENADO=C\nMD5_ARCHIVO=\n";
		txt_write_in_file(fd_auxiliar, cadena_auxiliar);
		fclose (fd_auxiliar);

		fd_auxiliar = fopen(path_basura_metadata, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		cadena_auxiliar = "SIZE=0\nBLOCK_COUNT=0\nBLOCKS=[]\nCARACTER_LLENADO=C\nMD5_ARCHIVO=\n";
		txt_write_in_file(fd_auxiliar, cadena_auxiliar);
		fclose (fd_auxiliar);

		//Creacion File Blocks
		fd_auxiliar = fopen(path_blocks, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		cadena_auxiliar = string_repeat(' ',iCantidadTotalDeBytesEnArchivoBlocks);
		txt_write_in_file(fd_auxiliar, cadena_auxiliar);
		fclose (fd_auxiliar);

		//Creacion File Superbloque
		cadena_auxiliar = string_repeat('0',iCantidadBloques);
		//Asigna la estructura del superbloque
		str_FileSupeBloque.cantidad_bloques = iCantidadBloques;
		str_FileSupeBloque.tamanio_bloque = iTamanioBloque;
//		str_FileSupeBloque.bitmap = *bitarray_create(cadena_auxiliar, iCantidadBloques);

		fd_auxiliar = fopen(path_superbloque, "w");
		if (fd_auxiliar == NULL) {
			perror("Hubo un error al abrir el arhivo");
			exit(-1);
		}
		fwrite(&str_FileSupeBloque, sizeof(str_FileSupeBloque), 1, fd_auxiliar);
		fclose (fd_auxiliar);


		//TODO Crear la carpeta de bitacoras de cero


	}

	return 0;

}
