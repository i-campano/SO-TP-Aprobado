/*
 */

#include "IMongoStore.h"
#include "bitacora.h"
#include "utils.h"
#include "sabotaje.h"

int main(void)
{


	signal(SIGUSR1,informarSabotaje);
	signal(SIGSEGV,adulterar_bitmap);
	iniciar_configuracion();
	remove_files();



	iniciar_super_block();

	iniciar_blocks();

	iniciar_archivo(conf_ARCHIVO_OXIGENO_NOMBRE,&archivo_oxigeno, "oxigeno","O");
	iniciar_archivo(conf_ARCHIVO_COMIDA_NOMBRE,&archivo_comida,"comida","C");
	iniciar_archivo(conf_ARCHIVO_BASURA_NOMBRE,&archivo_basura,"basura","B");
//	prueba_func_core_ejecucion();

	fs_server = iniciarServidor(5003);

	hilo_sincronizar_blocks();

	manejadorDeHilos();



	//	En el server cuando atiendo a los tripulantes crear a demanda los archivos para bitacora
//	manejadorDeHilos();

//	prueba_func_core_ejecucion();

	while(1);




	munmap(_blocks.fs_bloques, superblock.cantidad_bloques*superblock.tamanio_bloque);

	close(_blocks.file_blocks);

	log_info(logger,"TERMINO TODO OK");
	return EXIT_SUCCESS;
}

void informarSabotaje(int signal){
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
		sendRemasterizado(socketDiscordiador, 179, tamanio, (void*) buffer);
		free(posicion);
		sabotajes_realizados++;
}





void remove_files(){
	for(int i = 0; i<100; i++){
		char *resto_path = string_from_format("bitacora/tripulante_%d%s",i,".ims");
		   if (remove(resto_path) == 0)
		      log_info(logger,"Deleted successfully %d",i);
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
















