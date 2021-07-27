/*
 */

#include "IMongoStore.h"
#include "bitacora.h"
#include "utils.h"
#include "sabotaje.h"


int main(void)
{


	signal(SIGUSR1,informarSabotaje);
//	signal(SIGSEGV,adulterar_bitmap);
//	signal(SIGSEGV,terminar_imongo);
	iniciar_configuracion();
//	remove_files();

	check_directories_permissions(conf_PUNTO_MONTAJE);
	check_directorios();


	iniciar_super_block();

	iniciar_blocks();

//	crear_bitacora();

	create_metadata_resource_files();
	check_metadata_resources();


	delete_bitacora_files(conf_PUNTO_MONTAJE);




	fs_server = iniciarServidor(conf_PUERTO_IMONGO);

	hilo_sincronizar_blocks();

	log_debug(logger,"SYNC: superblocks.ims: %d",*(uint32_t*)superblock.bitmapstr);
	log_debug(logger,"SYNC: superblocks.ims: %d",*(uint32_t*)(superblock.bitmapstr+sizeof(uint32_t)));
	manejadorDeHilos();


	munmap(_blocks.fs_bloques, superblock.cantidad_bloques*superblock.tamanio_bloque);

	close(_blocks.file_blocks);

	log_info(logger,"TERMINO TODO OK");
	return EXIT_SUCCESS;
}


void informarSabotaje(int signal){
	//TODO: DESCOMENTAR FUNCION Y SACAR FSCK
	//	_informar_sabotaje_a_discordiador();
	fsck();
}

void _informar_sabotaje_a_discordiador(){
		char * posicion = string_new();
		string_append(&posicion,conf_POSICIONES_SABOTAJE[sabotajes_realizados]);
		log_info(logger, "Informando sabotaje al discordiador");
		log_info(logger, "%s",posicion);
		int largoClave = string_length(posicion);
		int tamanio = 0;
		//En el buffer mando clave y luego valor
		void* buffer = malloc(string_length(posicion) + sizeof(uint32_t));
		memcpy(buffer + tamanio, &largoClave, sizeof(uint32_t));
		tamanio += sizeof(uint32_t);
		memcpy(buffer + tamanio, posicion, string_length(posicion));
		tamanio += largoClave;
		sendRemasterizado(socketDiscordiador, INFORMAR_SABOTAJE, tamanio, (void*) buffer);
		free(posicion);
		sabotajes_realizados++;
}


void remove_files(){
	for(int i = 0; i<100; i++){
		//TODO : quitar el eliminar --- o en su defecto sacar el 100 del for
		char *resto_path = string_from_format("bitacora/tripulante_%d%s",i,".ims");
		   if (remove(resto_path) == 0)
		      log_info(logger,"Deleted successfully %d",i);
		   free(resto_path);
	}
	log_info(logger,"Finished deleted files");
}

void prueba_func_core_ejecucion2(){
	int libres = calcular_bloques_libres();
	log_info(logger,"libres = %d",libres);

	write_archivo("1244",archivo_oxigeno);

	consumir_arch(archivo_oxigeno,3);



	libres = calcular_bloques_libres();
	log_info(logger,"libres = %d",libres);
}


void prueba_func_core_ejecucion(){
	int libres = calcular_bloques_libres();
	log_info(logger,"libres = %d",libres);

	_archivo_bitacora * archivo = iniciar_archivo_bitacora("tripulante1","tarea1");

	_archivo_bitacora * archivo2 = iniciar_archivo_bitacora("tripulante2","tarea1");

	write_archivo_bitacora("hola",archivo);

	write_archivo_bitacora("hola3",archivo2);



	libres = calcular_bloques_libres();
	log_info(logger,"libres = %d",libres);
}



void check_directories_permissions(char *mount_point) {
	// chequeo que el punto de montaje exista y tenga permiso
	log_info(logger, "Chequeando punto de montaje %s", mount_point);

	log_debug(logger, "Chequeando directorio %s", mount_point);

	if (access(mount_point, R_OK | W_OK) != 0) {
		log_error(logger, "No se puede acceder al directorio %s", mount_point);
		exit(0);
	}

//	check_readable_file(mount_point, FILE_METADATA);

}

void delete_bitacora_files(char *basedir) {
	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	path_files = string_duplicate(conf_PATH_BITACORA);
	string_append_with_format(&aux, "%s", path_files);
	char * command = string_new();
	string_append_with_format(&command, "rm -r %s*", aux);

	system(command);
	log_warning(logger, "El directorio bitacora esta limpio");

	//TODO LIMPIAR BITMAP¿

	free(path_files);
	free(command);
	free(aux);
}

void check_files_access(char *basedir, char *ask) {
	char *aux = NULL;
	char *path_files = NULL;
	aux = string_duplicate(conf_PUNTO_MONTAJE);
	path_files = string_duplicate(conf_PATH_FILES);
	string_append_with_format(&aux, "/%s%s", path_files,ask);

	if (access(aux, R_OK | W_OK) != 0) {
		log_error(logger, "No se puede acceder al directorio %s", aux);
		free(aux);
		exit(EXIT_FAILURE);
	}
	free(aux);
}

void check_directory(char *basedir, char *ask) {
	char *aux = NULL;
	aux = string_duplicate(basedir);
	string_append_with_format(&aux, "/%s", ask);

	if (access(aux, R_OK | W_OK) != 0) {
		log_error(logger, "No se puede acceder al directorio %s", aux);
		free(aux);
		exit(EXIT_FAILURE);
	}
	free(aux);
}



void check_directorios() {
	check_directory(conf_PUNTO_MONTAJE, conf_PATH_FILES);
	check_directory(conf_PUNTO_MONTAJE, conf_PATH_BITACORA);
//	check_directory(conf_PUNTO_MONTAJE, conf_ARCHIVO_COMIDA_NOMBRE);
//	check_directory(conf_PUNTO_MONTAJE, conf_ARCHIVO_BASURA_NOMBRE);
//	check_directory(conf_PUNTO_MONTAJE, conf_ARCHIVO_OXIGENO_NOMBRE);
}

void check_metadata_resources() {
	check_files_access(conf_PUNTO_MONTAJE, conf_ARCHIVO_COMIDA_NOMBRE);
	check_files_access(conf_PUNTO_MONTAJE, conf_ARCHIVO_BASURA_NOMBRE);
	check_files_access(conf_PUNTO_MONTAJE, conf_ARCHIVO_OXIGENO_NOMBRE);
}

void create_metadata_resource_files() {
	iniciar_archivo(conf_ARCHIVO_OXIGENO_NOMBRE, &archivo_oxigeno, "oxigeno","O");
	iniciar_archivo(conf_ARCHIVO_COMIDA_NOMBRE, &archivo_comida, "comida", "C");
	iniciar_archivo(conf_ARCHIVO_BASURA_NOMBRE, &archivo_basura, "basura", "B");
}










