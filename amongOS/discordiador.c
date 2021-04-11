#include "discordiador.h"

int main(void) {
	// PARTE 2
	t_log* logger = iniciar_logger();
	log_info(logger, "Soy el Discordiador");

	t_config* config = leer_config();
	char* valor = config_get_string_value(config, "CLAVE");
	log_info(logger, valor);

	char* ip = config_get_string_value(config, "IP");
	log_info(logger, ip);
	leer_consola(logger);

	// PARTE 3
	int conexion = crear_conexion(
		config_get_string_value(config, "IP"),
		config_get_string_value(config, "PUERTO")
	);


	enviar_mensaje(valor, conexion);

	t_paquete* paquete = armar_paquete();

	enviar_paquete(paquete, conexion);

	// Viejo terminar programa
	eliminar_paquete(paquete);
	log_destroy(logger);
	config_destroy(config);
	close(conexion);
}

t_log* iniciar_logger() {
	t_log* logger;

	if( (logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO))==NULL){
		printf("No se pudo crear el logger. Revise parametros\n");
		exit(1);
	}
	return logger;
}

t_config* leer_config() {
	return config_create("discordiador.config");
}

void leer_consola(t_log* logger) {
	char* leido = readline(">");

	while(strncmp(leido, "", 1) != 0) {
		log_info(logger, leido);
		free(leido);
		leido = readline(">");
	}

	free(leido);
}

t_paquete* armar_paquete() {
	t_paquete* paquete = crear_paquete();

	char* leido = readline(">");

	while(strncmp(leido, "", 1) != 0) {
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
		free(leido);
		leido = readline(">");
	}

	free(leido);

	return paquete;
}
