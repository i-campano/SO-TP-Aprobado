/*
 */

#include "IMongoStore.h"

int main(void)
{
		pthread_mutex_init(&mut_ARCHIVO_OXIGENO, NULL);
		pthread_mutex_init(&mut_ARCHIVO_COMIDA, NULL);
		pthread_mutex_init(&mut_ARCHIVO_BASURA, NULL);

	//Inicializacion del loger
	logger = log_create("IMongoStore.log", "IMongoStore", 1, LOG_LEVEL_DEBUG);

	//Inicializacion del archivo de Configuracion
	iniciar_configuracion();

	server_fd = iniciarServidor(conf_PUERTO_IMONGO);

	//iniciarEstructurasAdministrativas();

	log_info(logger, "Servidor listo para recibir al cliente");

	//Inicia escucha de llamados
	//manejadorDeHilos();
	escribirBitacora("se movio de (1,1) a (1,2)",1);
	generarOxigeno(5);
	generarBasura(5);
	generarComida(5);
	consumirOxigeno(2);
	consumirComida(3);
	descartarBasura();



	log_info(logger,"finished");

/*	generarOxigeno(10);
	consumirOxigeno(5);

	generarBasura(8);
	descartarBasura();

	ejecutarTarea("GENERAR_OXIGENO",10);
	ejecutarTarea("GENERAR_BASURA",10);*/


	return EXIT_SUCCESS;
}
