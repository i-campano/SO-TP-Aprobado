#include "discordiador.h"

int main(void) {
	iniciar_logger();
	//Inicia las colas de planificacion
	iniciarEstructurasAdministrativasPlanificador();

	iniciar_configuracion();

	socketServerMiRam = conectarAServer(ip_miram, puerto_miram);

	realizarHandshake(socketServerMiRam, DISCORDIADOR, MIRAM);

	log_info(logger, "Planificador se conecto a MIRAM");

	socketServerIMongoStore = conectarAServer(ip_mongo, puerto_mongo);

	realizarHandshake(socketServerIMongoStore, DISCORDIADOR, IMONGOSTORE);

	log_info(logger, "Planificador se conecto a IMONGOSTORE");

	iniciarHiloConsola();

//	atenderIMongoStore();

	planificar();

	while(1){}

	return terminar_programa(logger,config,NULL);
}



