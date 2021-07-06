/*
 */

#include "IMongoStore.h"


int main(void)
{


	iniciar_configuracion();



	iniciar_super_block();

	iniciar_blocks();

	iniciar_archivo(conf_ARCHIVO_OXIGENO_NOMBRE,&archivo_oxigeno, "oxigeno","O");
//	iniciar_archivo(conf_ARCHIVO_COMIDA_NOMBRE,archivo_comida,"comida","C");
//	iniciar_archivo(conf_ARCHIVO_BASURA_NOMBRE,archivo_basura,"basura","B");
//	prueba_func_core_ejecucion();

	init_server();
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


void prueba_func_core_ejecucion(){
	int libres = calcular_bloques_libres();
	log_info(logger,"libres = %d",libres);

	write_archivo("1244",archivo_oxigeno);

	consumir_arch(archivo_oxigeno,3);



	libres = calcular_bloques_libres();
	log_info(logger,"libres = %d",libres);
}

















