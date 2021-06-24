/*
 */

#include "IMongoStore.h"

void inicializarMutex() {
	pthread_mutex_init(&mut_ARCHIVO_OXIGENO, NULL);
	pthread_mutex_init(&mut_ARCHIVO_COMIDA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_BASURA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_OXIGENO_METADATA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_COMIDA_METADATA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_BASURA_METADATA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_BLOCKS, NULL);
	pthread_mutex_init(&mut_ARCHIVO_SUPERBLOQUE, NULL);
}

int main(void)
{
	inicializarMutex();
	//Inicializacion del loger
	logger = log_create("IMongoStore.log", "IMongoStore", 1, LOG_LEVEL_DEBUG);

	//Inicializacion del archivo de Configuracion
	iniciar_configuracion();

	server_fd = iniciarServidor(conf_PUERTO_IMONGO);

	//iniciarEstructurasAdministrativas();

	log_info(logger, "Servidor listo para recibir al cliente");

	//Inicia escucha de llamados
	//manejadorDeHilos();

	iniciaEstructuraDeArchivos();

	/*escribirBitacora("resuelve sabotaje",1);
	escribirBitacora("resuelve sabotaje",2);
	escribirBitacora("resuelve sabotaje",3);
	escribirBitacora("se mueve saraza22",5);*/


	log_info(logger,"finished");

/*	generarOxigeno(10);
	consumirOxigeno(5);

	generarBasura(8);
	descartarBasura();

	ejecutarTarea("GENERAR_OXIGENO",10);
	ejecutarTarea("GENERAR_BASURA",10);*/


	return EXIT_SUCCESS;
}
