/*
 */

#include "IMongoStore.h"


int main(void)
{


	iniciar_configuracion();

	init_server();

	iniciar_super_block();

	iniciar_blocks();

	iniciar_archivo(conf_ARCHIVO_OXIGENO_NOMBRE,&archivo_oxigeno, "oxigeno");
//	iniciar_archivo(conf_ARCHIVO_COMIDA_NOMBRE,&archivo_comida,"comida");
//	iniciar_archivo(conf_ARCHIVO_BASURA_NOMBRE,&archivo_basura,"basura");

	write_archivo("123456789123",&archivo_oxigeno);
	write_archivo("45",&archivo_oxigeno);
	write_archivo("67",&archivo_oxigeno);


//	leer_contenido_archivo("ads",&archivo_oxigeno);

	consumir_arch(&archivo_oxigeno,10);

	write_archivo("78",&archivo_oxigeno);

	//	En el server cuando atiendo a los tripulantes crear a demanda los archivos para bitacora
//	manejadorDeHilos();

//	prueba_func_core_ejecucion();


	munmap(_blocks.fs_bloques, superblock.cantidad_bloques*superblock.tamanio_bloque);

	close(_blocks.file_blocks);

	log_info(logger,"TERMINO TODO OK");
	return EXIT_SUCCESS;
}


//void prueba_func_core_ejecucion(){
//	int libres = calcular_bloques_libres();
//	log_info(logger,"libres = %d",libres);
//
//	write_archivo("12345678",archivo_comida);
//
//	leer_contenido_archivo("ads",&archivo_comida);
//
//	leer_metadata_archivo("asd",1,archivo_comida);
//
//	consumir_arch(&archivo_comida,7); // borro 7
//
//
//	write_archivo("8",archivo_comida);
//
//
//	leer_contenido_archivo("ads",&archivo_comida);
//
//	leer_metadata_archivo("asd",1,archivo_comida);
//
//
//	consumir_arch(&archivo_comida,1); // borro 1
//
//
//	leer_contenido_archivo("ads",&archivo_comida);
//
//	leer_metadata_archivo("asd",1,archivo_comida);
//
//
//	libres = calcular_bloques_libres();
//	log_info(logger,"libres = %d",libres);
//}

















