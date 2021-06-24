#include"utils.h"

#include "socket.h"

void iniciarEstructurasAdministrativas(){
	lista_pcb = list_create();
	lista_tcb = list_create();

	pthread_mutex_init(&pthread_mutex_tcb_list,NULL);

	pthread_mutex_init(&pthread_mutex_pcb_list,NULL);
}


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
		pthread_create( thread_id , &attr, (void*) atenderNotificacion , (void*) pcclient);
		pthread_detach(thread_id);

	}

	//Chequeo que no falle el accept
}

void enviar_tarea(int socket, char * tarea) {
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

t_tripulante * encontrar_trip(int id_trip){
	bool encontrarTripulante(t_tripulante * tripulante){
		return tripulante->id == id_trip;
	}

	pthread_mutex_lock(&pthread_mutex_tcb_list);
	t_tripulante * tripulante = list_find(lista_tcb,(void*) encontrarTripulante);
	log_info(logger,"ENCONTRE EL TRIP EN TCB LIST: %d socket: %d", tripulante->id, tripulante->socket);
	pthread_mutex_unlock(&pthread_mutex_tcb_list);

	return tripulante;
}

pcb * encontrar_patota(int id_patota){
	bool encontrarPatota(pcb * pcb){
		return pcb->patotaid == id_patota;
	}

	pthread_mutex_lock(&pthread_mutex_pcb_list);
	pcb * pcb = list_find(lista_pcb,(void*) encontrarPatota);
	log_info(logger,"ENCONTRE PATOTA EN PCB LIST: %d", pcb->patotaid);
	pthread_mutex_unlock(&pthread_mutex_pcb_list);

	return pcb;
}

void *atenderNotificacion(void * paqueteSocket){

	int socket = *(int*)paqueteSocket;

	t_tripulante * tripulante = malloc(sizeof(t_tripulante));
	while(1){

	uint32_t nroNotificacion = recvDeNotificacion(socket);
	log_info(logger,"recibo notificacion n# %d de socket %d",nroNotificacion, socket);

	switch(nroNotificacion){



		case DISCORDIADOR:{
			//Case para hacer HANDSHAKE = Chequear la conexion
			log_info(logger,"Se ha conectado el DISCORDIADOR");
			sendDeNotificacion(socket, MIRAM);
		}

			break;


		case CREAR_PATOTA:{

			crear_pcb(socket);

			log_info(logger, "----------------FIN PATOTA CREADA----------------");
			list_iterate(listaSegmentos,mostrarEstadoMemoria);
			sendDeNotificacion(socket, PATOTA_CREADA);
			break;
		}

		case TRIPULANTE:{
			break;
		}
		case PEDIR_TAREA:{
			uint32_t id_trip = recvDeNotificacion(socket);
			log_info(logger,"Pide tarea el id_tripulante: %d desde socket: %d",id_trip,socket);

			//encontrar_trip(id_trip);

			char * tarea = obtener_tarea(tripulante);
			log_info(logger,"tripulante id: %d, patota id: %d, tarea: %s indice tarea a pedir: %d",tripulante->id,tripulante->patota_id,tarea,tripulante->instrucciones_ejecutadas);

			//Enrealidad hay que crear un atributo Tareas ejecutadas y a ese sumarle 1
			tripulante->instrucciones_ejecutadas++;

			enviar_tarea(socket,tarea);
			//sendDeNotificacion(socket,85);


			break;

		}
		case CREAR_TRIPULANTE:{
			uint32_t trip_id = recvDeNotificacion(socket);
			uint32_t patota_id = recvDeNotificacion(socket);
			t_tripulante * trip = malloc(sizeof(t_tripulante));
			trip->id =trip_id;
			trip->socket = socket;
			list_add(lista_tcb,trip);
			tripulante->id = trip_id;
			tripulante->socket = socket;
			tripulante->ubi_x = 0;
			tripulante->ubi_y = 0;
			tripulante->instrucciones_ejecutadas = 0;
			tripulante->patota_id = (int)patota_id;
			crear_tripulante(trip_id,patota_id);
			sendDeNotificacion(socket, TRIPULANTE_CREADO);
			log_info(logger, "tcb creado");
			break;

		}

		case ACTUALIZAR_ESTADO_MIRAM:{
			uint32_t id_trip = recvDeNotificacion(socket);
			uint32_t estado = recvDeNotificacion(socket);
			sendDeNotificacion(socket, ESTADO_ACTUALIZADO_MIRAM);
			log_info(logger, "estado ACTUALIZADO tripulante: %d, estado: %d",id_trip,estado);

			break;

		}

		case ACTUALIZAR_UBICACION:{
			uint32_t id_trip = recvDeNotificacion(socket);
			uint32_t x = recvDeNotificacion(socket);
			uint32_t y = recvDeNotificacion(socket);

			encontrar_trip(id_trip);

			sendDeNotificacion(socket, UBICACION_ACTUALIZADA);


			log_info(logger, "estado ACTUALIZADO tripulante: %d, ubicacion-> X: %d ; Y: %d",id_trip,x,y);

			break;

		}
		case GET_PCB:{
			uint32_t id_trip = recvDeNotificacion(socket);
			log_info(logger,"%s",getPcb(id_trip).tareas);
			tcb_t temp = getTcb(id_trip);
			log_info(logger,"Id-> %i,x-> %i , y-> %i \n",temp.id,temp.x,temp.y);
			tcb_t temp2 = getTcb(id_trip+1);
			log_info(logger,"Id-> %i,x-> %i , y-> %i \n",temp2.id,temp2.x,temp2.y);
			printf("%i \n",memoria_libre());
			break;
		}

		default:
			log_warning(logger, "La conexion recibida es erronea");
			break;

	}
	}

	return 0;

}

char * obtener_tarea(t_tripulante * tripulante){
	int patota_id = tripulante->patota_id;

	pcb * pcb = encontrar_patota(patota_id);

	char * tarea =string_new();
	log_info(logger,"tareas ejecutadas %d: tareas totales : %d",tripulante->instrucciones_ejecutadas,list_size(pcb->tareas_list));

	if(tripulante->instrucciones_ejecutadas<list_size(pcb->tareas_list)){
		string_append(&tarea,((t_tarea *)list_get(pcb->tareas_list,tripulante->instrucciones_ejecutadas))->nombre_tarea);
		log_info(logger,"TAREA: %s",tarea);

	}else{
		log_info(logger,"NO HAY MAS TAREAS: %s",tarea);
		string_append(&tarea,"--");
	}
	//char * tarea = "GENERAR_OXIGENO 12;2;3;5";
	return tarea;
}


void crear_pcb(int socket) {
	char* tareas = recibirString(socket);
	char ** arrayTareas = string_split(tareas,"-");
	t_list * lista = list_create();
	for(int i = 0; arrayTareas[i]!=NULL; i++){
		log_info(logger,"%s indice: %d",arrayTareas[i],i);

		t_tarea * tarea = malloc(sizeof(tarea));
		tarea->nombre_tarea = string_new();
		tarea->nombre_tarea = string_duplicate(arrayTareas[i]);

		string_append(&tarea,arrayTareas[i]);
		list_add(lista,(void*)tarea);
	}


	char* id_posiciones = recibirString(socket);
	uint32_t patotaid = recibirUint(socket);
	uint32_t cantidad_patota = recibirUint(socket);
	pcb* pcb = malloc(sizeof(pcb));
	pcb->id_posicion = string_new();
	pcb->cantidad_tripulantes = cantidad_patota;

	string_append(&pcb->id_posicion, id_posiciones);

	pcb->tareas_list = lista;


	pcb->patotaid = (int)patotaid;
	pcb->estado = 'N';
	pcb->socket_tcb = socket;
	pcb_t pcbRam;
	pcbRam.id = patotaid;
	pcbRam.tareas = string_new();
	string_append(&pcbRam.tareas,tareas);
	log_info(logger, "agregando patota en lista_pcb");
	crear_patota2(pcbRam,id_posiciones);
	pthread_mutex_lock(&pthread_mutex_pcb_list);
	list_add(lista_pcb, pcb);
	pthread_mutex_unlock(&pthread_mutex_pcb_list);

	mostrar_patota(pcb);
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
/*void crear_pcb2(int socket) {
	char* tareas = recibirString(socket); //RECIBO TODAS LAS TAREAS JUNTAS
	char* id_posiciones = recibirString(socket);//RECIBO POSICIONES
	uint32_t patotaid = recibirUint(socket);//RECIBO ID
	uint32_t cantidad_patota = recibirUint(socket);//RECIBO CUANTOS TRIPS
	pcb_t pcb; //TIPO DE PATOTA ADMIN MIRAM
	pcb.id = patotaid;
	pcb.tareas = string_new();
	string_append(&pcb.tareas,tareas);
	crear_patota2(cantidad_patota,pcb,id_posiciones);

}*/
void mostrar_patota(pcb* pcb){

	log_info(logger,"PATOTA ID: %d - Posicion: %s - Cantidad trip: %d - Estado: %c",pcb->patotaid, pcb->id_posicion,pcb->cantidad_tripulantes,pcb->estado);
}

void mostrar_lista_patota(){

	log_info(logger,"LISTA DE PCB------------------------");
	pthread_mutex_lock(&pthread_mutex_pcb_list);
	list_iterate(lista_pcb, (void*) mostrar_patota);
	pthread_mutex_unlock(&pthread_mutex_pcb_list);


}

