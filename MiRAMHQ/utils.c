#include"utils.h"



int iniciar_servidor(void)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0){
		return cod_op;
	}
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

//podemos usar la lista de valores para poder hablar del for y de como recorrer la lista
t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

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


		case 2:{
			log_info(logger,"Se ha conectado el DISCORDIADOR");
			sendDeNotificacion(socket, 9);
		}

			break;

		case 3:{

			log_info(logger,"recibo string....");

			char* texto = recibirString(socket);
			log_info(logger,texto);

			char* texto2 = recibirString(socket);

			log_info(logger,texto2);

			uint32_t patotaid = recibirUint(socket);

			printf("%i\n",patotaid);

			uint32_t cantidad_patota = recibirUint(socket);

			printf("%i\n",cantidad_patota);


			break;
		}

		case 4:{

			log_info(logger,"recibo string....");

			char* texto = recibirString(socket);
			log_info(logger,texto);

			char* texto2 = recibirString(socket);

			log_info(logger,texto2);

			uint32_t patotaid = recibirUint(socket);

			printf("%i\n",patotaid);

			uint32_t cantidad_patota = recibirUint(socket);

			printf("%i\n",cantidad_patota);
        	uint32_t valor = 22;


			//printf("%i\n",valor);

			sendDeNotificacion(socket,valor);


			break;
		}
		default:
			log_warning(logger, "La conexion recibida es erronea");
			break;

	}
	}

	return 0;

}

