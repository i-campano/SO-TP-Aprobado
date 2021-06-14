#include "discordiador.h"

int main(void) {
	iniciar_logger();
	//Inicia las colas de planificacion

	iniciar_configuracion();

	iniciarEstructurasAdministrativasPlanificador();

	socketServerMiRam = conectarAServer(IP_MIRAM, PUERTO_MIRAM);

	realizarHandshake(socketServerMiRam, DISCORDIADOR, MIRAM);

	log_info(logger, "Planificador se conecto a MIRAM");

	socketServerIMongoStore = conectarAServer(IP_MONGO, PUERTO_MONGO);

	realizarHandshake(socketServerIMongoStore, DISCORDIADOR, IMONGOSTORE);

	log_info(logger, "Planificador se conecto a IMONGOSTORE");

	iniciarHiloConsola();

//	atenderIMongoStore();

	planificar();

	while(1){}

	return terminar_programa(logger,config,NULL);
}



