#include"utils.h"

#include "socket.h"


void iniciarEstructurasAdministrativas(){
	lista_pcb = list_create();
	lista_tcb = list_create();
}


void manejadorDeHilos(){
	int socketCliente;

	// Funcion principal
	while((socketCliente = aceptarConexionDeCliente(server_fd))) { 	// hago el accept
		log_info(logger,"se conecto: %d", socketCliente);
		pthread_t * thread_id = malloc(sizeof(pthread_t));
    	pthread_attr_t attr;
    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    	int * pcclient = malloc(sizeof(int));
    	*pcclient = socketCliente;
		//Creo hilo atendedor
    	//sleep(2);
		pthread_create( thread_id , &attr, (void*) atenderNotificacion , (void*) pcclient);
		pthread_detach(thread_id);

	}

	//Chequeo que no falle el accept
}

void enviar_tarea(int socket, char * tarea) {
	log_info(logger, "entro a  pedir TAREA");
	char* claveNueva = tarea;
	int largoClave = string_length(claveNueva);
	int tamanio = 0;
	//En el buffer mando clave y luego valor
	void* buffer = malloc(string_length(claveNueva) + sizeof(uint32_t));
	memcpy(buffer + tamanio, &largoClave, sizeof(uint32_t));
	tamanio += sizeof(uint32_t);
	memcpy(buffer + tamanio, claveNueva, string_length(claveNueva));
	tamanio += largoClave;
	sendRemasterizado(socket, ENVIAR_TAREA, tamanio, (void*) buffer);
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
			sendDeNotificacion(socket, MIRAM);
		}

			break;


		case CREAR_PATOTA:{

			crear_pcb(socket);
			log_info(logger, "----------------PATOTAS EN MI RAM: ");
			mostrar_lista_patota();
			log_info(logger, "----------------FIN PATOTA CREADA----------------");
			sendDeNotificacion(socket, PATOTA_CREADA);
			break;
		}

		case TRIPULANTE:{
			log_info(logger, "----------------tcb creado----------------");
			break;
		}
		case PEDIR_TAREA:{
			uint32_t id_trip = recvDeNotificacion(socket);
			log_info(logger,"id_tripulante: %d",id_trip);
			log_info(logger,"se conecto socket: %d",socket);

			char * tarea = obtener_tarea(id_trip);

			enviar_tarea(socket,tarea);
			//sendDeNotificacion(socket,85);


			log_info(logger, "----------------tcb creado----------------");

			break;

		}
		default:
			log_warning(logger, "La conexion recibida es erronea");
			break;

	}
	}

	return 0;

}

char * obtener_tarea(int id_tripulante){
	char * tarea = "GENERAR_OXIGENO 12;2;3;5";
	return tarea;
}


void crear_pcb(int socket) {
	char* tareas = recibirString(socket);
	log_info(logger, tareas);
	char* id_posiciones = recibirString(socket);
	log_info(logger, id_posiciones);
	uint32_t patotaid = recibirUint(socket);
	log_info(logger, "patotaid: %d", (int) patotaid);
	uint32_t cantidad_patota = recibirUint(socket);
	log_info(logger, "cantidad tripulantes: %d", (int) cantidad_patota);
	tcb2* tcb = malloc(sizeof(tcb2));
	tcb->id_posicion = string_new();
	tcb->tareas = string_new();
	tcb->cantidad_tripulantes = cantidad_patota;
	strcpy(tcb->id_posicion, id_posiciones);
	strcpy(tcb->tareas, tareas);
	tcb->patotaid = patotaid;
	tcb->estado = 'N';
	tcb->socket_tcb = socket;
	log_info(logger, "agregando patota en lista_tcb");
	list_add(lista_pcb, tcb);

	//printf("%c\n",tcb->estado);

}

void crear_tcb(int socket) {


	//uint32_t patotaid = recibirUint(socket);
	//log_info(logger, "patotaid: %d", (int) patotaid);
	//uint32_t cantidad_patota = recibirUint(socket);
	//tcb2* tcb = malloc(sizeof(tcb2));
	//tcb->patotaid = patotaid;
	//tcb->estado = 'N';
	//tcb->socket_tcb = socket;
	//log_info(logger, "agregando patota en lista_tcb");
	//list_add(lista_tcb, tcb);

	//printf("%c\n",tcb->estado);

}


void mostrar_lista_patota(){
	int i = 0;
	void mostrar_patota(tcb2* tcb){
		i++;
		log_info(logger,"# N: %d - PATOTA ID: %d - Tareas: %s - Posicion: %s - Cantidad trip: %d - Estado: %c", i,tcb->patotaid,tcb->tareas, tcb->id_posicion,tcb->cantidad_tripulantes,tcb->estado);
	}

	list_iterate(lista_pcb, (void*) mostrar_patota);
}

