/*
 * conexiones.c
 */

#include"utils.h"



void manejadorDeHilos(){
	int socketCliente;

	// Funcion principal
	while((socketCliente = aceptarConexionDeCliente(server_fd))) { 	// hago el accept
		pthread_t * thread_id = malloc(sizeof(pthread_t));
    	pthread_attr_t attr;
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    	int * pcclient = malloc(sizeof(int));
    	*pcclient = socketCliente;
		//Creo hilo atendedor
    	sleep(4);
		pthread_create( thread_id , &attr, (void*) atenderNotificacion , (void*) pcclient);
		pthread_detach(thread_id);

	}

	//Chequeo que no falle el accept
}


void *atenderNotificacion(void * paqueteSocket){

	int socket = *(int*)paqueteSocket;

	while(1){

		log_info(logger,"espero mas notificaciones....");
		uint32_t nroNotificacion = recvDeNotificacion(socket);

		switch(nroNotificacion){

			log_info(logger,"en el switch....");


			case DISCORDIADOR:{
				//Case para hacer HANDSHAKE = Chequear la conexion
				log_info(logger,"Se ha conectado el DISCORDIADOR");
				sendDeNotificacion(socket, IMONGOSTORE);
				//while(1){
					//sleep(10);
					//log_info(logger,"ENVIANDO TAREAS");
					//sendDeNotificacion(socket, ACTUALIZACION_IMONGOSTORE);
				//}
			}
				break;

			case EJECUTAR_TAREA:{
				char * tarea = recibirString(socket);
				//Case para hacer HANDSHAKE = Chequear la conexion

				uint32_t id_trip = recvDeNotificacion(socket);
				ejecutar_tarea(tarea);
				sendDeNotificacion(socket,198);
				log_info(logger,"Id tripulante %d quiere hacer la tarea: %s",id_trip,tarea);

			}
				break;
			default:
				log_warning(logger, "La conexion recibida es erronea");
				break;
		}
	}
	return 0;
}

void ejecutar_tarea(char * tarea){
	if(strcmp(tarea,"GENERAR_OXIGENO 12;2;3;5")==0){
		log_info(logger,"EJECUTO TAREA - GENERAR OXIGENO:");
		log_info(logger,"OOOOOOOOOOO");
	}

}

