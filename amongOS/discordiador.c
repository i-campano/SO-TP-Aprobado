#include "discordiador.h"

int main(void) {
	iniciar_logger();
	//Inicia las colas de planificacion
	iniciarEstructurasAdministrativasPlanificador();

	t_config* config = leer_config();

	char* valor = config_get_string_value(config, "CLAVE");

	char* ip = config_get_string_value(config, "IP");

	char * valorip = "127.0.0.1";

	iniciarHiloConsola();

	socketServerMiRam = conectarAServer(ip, 5002);

	realizarHandshake(socketServerMiRam, DISCORDIADOR, MIRAM);
	log_info(logger, "Planificador se conecto a MIRAM");

	socketServerIMongoStore = conectarAServer(valorip, 5003);


	//realizarHandshake(socketServerIMongoStore, DISCORDIADOR, IMONGOSTORE);

	//log_info(logger, "Planificador se conecto a IMONGOSTORE");

	//atenderIMongoStore();

	planificar();

	while(1){}

	return terminar_programa(logger,config,NULL);
}



