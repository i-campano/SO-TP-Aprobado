#include "discordiador.h"

unsigned int contador_hilos = 0;

int main(void) {
	iniciar_logger();
	//Inicia las colas de planificacion
	iniciarEstructurasAdministrativasPlanificador();

	t_config* config = leer_config();

	char* valor = config_get_string_value(config, "CLAVE");

	char* ip = config_get_string_value(config, "IP");

	char * valorip = "127.0.0.1";

	iniciarHiloConsola();

	socketServerMiRam = conectarAServer(valorip, 5002);

	//realizarHandshake(socketServerMiRam, DISCORDIADOR, MIRAM);

	socketServerIMongoStore = conectarAServer(valorip, 5003);

	//log_info(logger, "Planificador se conecto a MIRAM");

	//realizarHandshake(socketServerIMongoStore, DISCORDIADOR, IMONGOSTORE);

	//log_info(logger, "Planificador se conecto a IMONGOSTORE");

	//atenderIMongoStore();

	planificar();

	while(1){}

	return terminar_programa(logger,config,NULL);
}






//-------------------------------------MOVER A OTRO ARCHIVO--------------------------------------------------------------------


int terminar_programa(t_log* logger,t_config* config,int conexion[2]) {
	log_destroy(logger);
	config_destroy(config);
	return 0;
}

void iniciar_logger() {
	if( (logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO))==NULL){
		printf("No se pudo crear el logger. Revise parametros\n");
		exit(1);
	}
}

t_config* leer_config() {
	t_config *config;
	if((config = config_create("discordiador.config"))==NULL) {
		printf("No se pudo leer de la config. Revise. \n");
		exit(1);
	}
	return config;
}


void leer_consola2(t_log* logger,int conexion_ram,int conexion_fs,int conexion_trip) {
	char* leido = readline(">");
	realizar_operacion(leido,conexion_ram,conexion_fs,conexion_trip);
	while(strncmp(leido, "", 1) != 0) {
		log_info(logger, leido);
		free(leido);
		leido = readline(">");
		realizar_operacion(leido,conexion_ram,conexion_fs,conexion_trip);
	}

	free(leido);
}

//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------SIN USO - REVISAR----------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------


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
			break;
		}
		case PAUSAR_PLANIFICACION: {
			//Realizar Tarea acorde a INICIAR
			break;
		}
		case REANUDAR_PLANIFICACION: {
			break;
		}
		case LISTAR_TRIPULANTES: {
			break;
		}
		case EXPULSAR_TRIPULANTE: {
			break;
		}
		case BITACORA_TRIPULANTE :{
			break;
		}
		case INICIAR_PATOTA :{
			//inicializar_patota(mensaje);
			break;
		}
		default: {
			return -1;
		}
	}
	return 0;

}


void *labor_tripulante (void* tripulante) {

//	tripulante_t* p_aux_trip;
//	p_aux_trip = (tripulante_t *) tripulante;

//	while (!(p_aux_trip->fin_tareas)) {
//
//		switch (p_aux_trip->estado){
//			case NEW: {
//				if (p_aux_trip->tarea != NULL){
//					//Si ya se mi tarea no la pido otra vez
//					break;
//				}
//				//Pedir tarea a mi Ram
//				//El planificador vera si estoy ready o no
//				break;
//			}
//			case READY: {
//				//Literalmente no hago nada en ready quizas informar donde estoy por algun LOG no se
//				break;
//			}
//			case EXEC: {
//				p_aux_trip->trabajando = TRUE;
//
//				//Me pongo a laburar Conectarse al MONGO y realizar los cambios en las tareas INFORMO A MI RAM
//				//Moverme a X Y posicion e informar a MI_RAM y Mongo que hice
//
//
//				//Pido siguiente tarea a MIRAM y lo guardo en *ẗarea
//				if (p_aux_trip->tarea == NULL){
//					//Termine tareas porque no hay mas
//					p_aux_trip->fin_tareas = TRUE;
//					p_aux_trip->trabajando = FALSE; //Porque ya termine
//				}
//				p_aux_trip->trabajando = FALSE; //Porque ya termine
//				//Se puede buguear!! ojo si el planificador es mas lento, muy poco posible mirando el codigo
//				break;
//			}
//			case FIN: {
//				//No se si informar aca en los logs o afuera en el planif aca se piensa
//				return NULL; //LLEGUE AL FIN TERMINO HILO
//			}
//		}
//	}

	return NULL;
}
