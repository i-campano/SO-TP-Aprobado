/*
 * conexiones.c
 */

#include"utils.h"



void manejadorDeHilos(){
	int socketCliente;

	// Funcion principal
	while((socketCliente = aceptarConexionDeCliente(server_fd))) { 	// hago el accept

		pthread_t thread_id;
    	pthread_attr_t attr;
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		//Creo hilo atendedor
		pthread_create( &thread_id , &attr, (void*) atenderNotificacion , (void*) &socketCliente);

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
				while(1){
					sleep(10);
					log_info(logger,"ENVIANDO TAREAS");
					sendDeNotificacion(socket, ACTUALIZACION_IMONGOSTORE);
				}
			}
				break;
			default:
				log_warning(logger, "La conexion recibida es erronea");
				break;
		}
	}
	return 0;
}

