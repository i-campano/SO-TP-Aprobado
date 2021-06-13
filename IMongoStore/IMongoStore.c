/*
 */

#include "IMongoStore.h"

int main(void)
{

	//Inicializacion del loger
	logger = log_create("IMongoStore.log", "IMongoStore", 1, LOG_LEVEL_DEBUG);

	//Inicializacion del archivo de Configuracion
	iniciar_configuracion();

	server_fd = iniciarServidor(5003);

	//iniciarEstructurasAdministrativas();

	log_info(logger, "Servidor listo para recibir al cliente");
	manejadorDeHilos();

	return EXIT_SUCCESS;
}
