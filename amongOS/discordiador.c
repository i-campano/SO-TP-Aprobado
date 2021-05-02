#include "discordiador.h"

int main(void) {

	t_log* logger = iniciar_logger();
	log_info(logger, "Soy el Discordiador");

	t_config* config = leer_config();
	char* valor = config_get_string_value(config, "CLAVE");
	log_info(logger, valor);

	char* ip = config_get_string_value(config, "IP");
	log_info(logger, ip);
	int conexion_ram = crear_conexion(ip,config_get_string_value(config, "PUERTO"));

	leer_consola(logger,conexion_ram,conexion_ram,conexion_ram);

	// Viejo terminar programa
	log_destroy(logger);
	config_destroy(config);
	close(conexion_ram);
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
	t_config *config;
	if((config = config_create("discordiador.config"))==NULL) {
		printf("No se pudo leer de la config. Revise. \n");
		exit(1);
	}
	return config;
}

void leer_consola(t_log* logger,int conexion_ram,int conexion_fs,int conexion_trip) {
	char* leido = readline(">");
	realizar_operacion(leido,conexion_ram,conexion_fs,conexion_trip);
	while(strncmp(leido, "", 1) != 0) {
		log_info(logger, leido);
		printf("Entre en WHILE LEER \n");
		free(leido);
		leido = readline(">");
		realizar_operacion(leido,conexion_ram,conexion_fs,conexion_trip);
	}

	free(leido);
}

int realizar_operacion(char* mensaje,int conexion_mi_ram,int conexion_file_system,int conexion_tripulante) {
	int codigo_operacion;

	if(mensaje == NULL) {
		return -1;
	}
	codigo_operacion = reconocer_op_code(mensaje);
	printf("Cod OP %d \n",codigo_operacion);
	switch (codigo_operacion) {
		case INICIAR_PLANIFICACION: {
			//Accion iniciar
			enviar_mensaje("Inicio_Planificacion\0",conexion_mi_ram);
			break;
		}
		case PAUSAR_PLANIFICACION: {
			//Realizar Tarea acorde a INICIAR
			enviar_mensaje("Pausar_Planificacion",conexion_mi_ram);
			break;
		}
		case REANUDAR_PLANIFICACION: {
			enviar_mensaje("Reanudar_planificacion",conexion_mi_ram);
			break;
		}
		case NEW_TRIPULANTE: {
			//PARSEO DEL MENSAJE
			enviar_mensaje("Iniciar_tripulante",conexion_mi_ram);
			break;
		}
		case LISTAR_TRIPULANTES: {
			enviar_mensaje("Listar_Trip",conexion_file_system);
			break;
		}
		case EXPULSAR_TRIPULANTE: {
			enviar_mensaje("Hay_que_rajar_a_1",conexion_mi_ram);
			break;
		}
		case BITACORA_TRIPULANTE :{
			enviar_mensaje("Quiero_la_bitacora",conexion_file_system);
			break;
		}
		default: {
			return -1;
		}
	}
	return 0;

}


