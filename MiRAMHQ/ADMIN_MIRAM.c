/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: AFIRM
 */
#include "ADMIN_MIRAM.h"

void* mem_ppal2 = NULL;
void* mem_ppal = NULL;
bool algoritmo = FF;
uint32_t tamanio = 1024;

uint32_t tamanioPagina = 64;
uint32_t tamanioMemoria = 1024;

//PRUEBAS -->
//uint32_t id_patota = 1;
//uint32_t id_trip = 1;

//UTILIZADAS EN ITERACION DEL DUMP SEGMENTACION
//PARA CREAR TAREAS
//uint32_t tamanioTareas = 0;

int admin_memoria(void)
{
	paginacion = true;
	pthread_mutex_init(&accesoMemoria,NULL);
	pthread_mutex_init(&accesoListaSegmentos,NULL);
	crear_memoria_();

	//free(mem_ppal);
	return 0;
}



//Criterios para listas de segmentos

bool condicionSegmentoLibrePcb(void* segmento) {
	segmento_t* segmento_tmp = (segmento_t*)segmento;
	uint32_t tamanioTotal = sizeof(pcb_t);
	return (segmento_tmp->fin - segmento_tmp->inicio) >= tamanioTotal && segmento_tmp->id == 0;
}
void* condicionSegmentoLibrePcbBF(void* segmento,void* otroSegmento) {
	segmento_t* segmento_tmp = (segmento_t*)segmento;
	segmento_t* segmento_tmp2 = (segmento_t*)otroSegmento;
	uint32_t tamanioTotal = sizeof(pcb_t);
	uint32_t tamanio1 = segmento_tmp->fin - segmento_tmp->inicio;
	uint32_t tamanio2 = segmento_tmp2->fin - segmento_tmp2->inicio;
	if(segmento_tmp->tipoDato != VACIO){
		return otroSegmento;
	}
	if(segmento_tmp2->tipoDato != VACIO && segmento_tmp->tipoDato == VACIO){
		return segmento;
	}
	if(tamanio1 < tamanioTotal) {
		return otroSegmento;
	}
	if(tamanio2 < tamanioTotal){
		return segmento;
	}
	if(tamanio1 > tamanio2) {
		return otroSegmento;
	}
	return segmento;
}
void* condicionSegmentoLibreTcbBF(void* segmento,void* otroSegmento) {
	segmento_t* segmento_tmp = (segmento_t*)segmento;
	segmento_t* segmento_tmp2 = (segmento_t*)otroSegmento;
	uint32_t tamanioTotal = sizeof(tcb_t);
	uint32_t tamanio1 = segmento_tmp->fin - segmento_tmp->inicio;
	uint32_t tamanio2 = segmento_tmp2->fin - segmento_tmp2->inicio;
	if(segmento_tmp->tipoDato != VACIO){
		return otroSegmento;
	}
	if(segmento_tmp2->tipoDato != VACIO && segmento_tmp->tipoDato == VACIO){
		return segmento;
	}
	if(tamanio1 < tamanioTotal) {
		return otroSegmento;
	}
	if(tamanio2 < tamanioTotal){
		return segmento;
	}
	if(tamanio1 > tamanio2) {
		return otroSegmento;
	}
	return segmento;
}
bool condicionSegmentoLibreTcb(void* segmento) {
	segmento_t* segmento_tmp = (segmento_t*)segmento;
	uint32_t tamanioTotal = sizeof(tcb_t);
	return (segmento_tmp->fin - segmento_tmp->inicio) >= tamanioTotal && segmento_tmp->id == 0;
}
bool ordenar_segun_inicio(void* primero,void* segundo){
	segmento_t* unSegmento = (segmento_t*)primero;
	segmento_t* otroSegmento = (segmento_t*)segundo;
	return unSegmento->inicio < otroSegmento->inicio;
}

//Devuelve segmento segun CONFIGURACION FF BF
segmento_t* segmentoPcb_segun(bool algoritmoBusqueda) {
	segmento_t* segmentoMemoria;
	if(list_size(listaSegmentos) == 1){
				segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibrePcb);
			}
			else{
				if(algoritmoBusqueda == FF) {
					segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibrePcb);
				}
				else {
					segmentoMemoria = list_get_minimum(listaSegmentos,condicionSegmentoLibrePcbBF);
				}
			}
	return segmentoMemoria;
}
segmento_t* segmentoTcb_segun(bool algoritmoBusqueda) {
	segmento_t* segmentoMemoria;
	if(list_size(listaSegmentos) == 1){
			segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibreTcb);
	}
	else{
		if(algoritmoBusqueda == FF) {
			segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibreTcb);
		}
		else {
			segmentoMemoria = list_get_minimum(listaSegmentos,condicionSegmentoLibreTcbBF);
		}
	}
	return segmentoMemoria;
}
segmento_t* segmentoTareas_segun(bool algoritmoBusaqueda, uint32_t tamanioTareas){
	bool condicionSegmentoLibreTareas(void* segmento) {
			segmento_t* segmento_tmp = (segmento_t*)segmento;
			return (segmento_tmp->fin - segmento_tmp->inicio) >= tamanioTareas && segmento_tmp->id == 0;
		}
		void* condicionSegmentoLibreTareasBF(void* segmento,void* otroSegmento) {
			segmento_t* segmento_tmp = (segmento_t*)segmento;
			segmento_t* segmento_tmp2 = (segmento_t*)otroSegmento;
			uint32_t tamanio1 = segmento_tmp->fin - segmento_tmp->inicio;
			uint32_t tamanio2 = segmento_tmp2->fin - segmento_tmp2->inicio;
			if(segmento_tmp->tipoDato != VACIO){
				return otroSegmento;
			}
			if(segmento_tmp2->tipoDato != VACIO && segmento_tmp->tipoDato == VACIO){
				return segmento;
			}
			if(tamanio1 < tamanioTareas) {
				return otroSegmento;
			}
			if(tamanio2 < tamanioTareas){
				return segmento;
			}
			if(tamanio1 > tamanio2) {
				return otroSegmento;
			}
			return segmento;
		}
		segmento_t* segmentoMemoria = NULL;
				if(list_size(listaSegmentos) == 1){
					segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibreTareas);
				}
				else{
					if(algoritmo == FF) {
						segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibreTareas);
					}
					else {
						segmentoMemoria = list_get_minimum(listaSegmentos,condicionSegmentoLibreTareasBF);
					}
				}
		return segmentoMemoria;
}

//Busca segmentos libres para cada tipo de dato
segmento_t* buscar_segmento(pcb_t pcb) {
		segmento_t* segmentoMemoria = segmentoPcb_segun(algoritmo);
		if(segmentoMemoria == NULL){
			pthread_mutex_lock(&accesoMemoria);
			compactar_memoria();
			pthread_mutex_unlock(&accesoMemoria);
			segmentoMemoria = segmentoPcb_segun(algoritmo);
		}
		if ((segmentoMemoria->fin - segmentoMemoria->inicio) == sizeof(pcb_t)) {
			segmentoMemoria->id = pcb.id;
			segmentoMemoria->tipoDato = DATO_PCB;
			return segmentoMemoria;
		}
		uint32_t tamanioTotal = sizeof(pcb_t);
		uint32_t fin_viejo;
		fin_viejo = segmentoMemoria->fin;
		segmentoMemoria->fin = tamanioTotal + segmentoMemoria->inicio; //YA CALCULE QUE ENTRA EN EL FIND
		segmentoMemoria->id = pcb.id;
		segmentoMemoria->tipoDato = DATO_PCB;
		crear_segmento(segmentoMemoria->fin,fin_viejo);
		return segmentoMemoria;
}
segmento_t* buscar_segmentoTcb(tcb_t tcb,uint32_t patotaId) {
	segmento_t* segmentoMemoria = segmentoTcb_segun(algoritmo);
		if(segmentoMemoria == NULL){
			pthread_mutex_lock(&accesoMemoria);
			compactar_memoria();
			pthread_mutex_unlock(&accesoMemoria);
			segmentoMemoria = segmentoTcb_segun(algoritmo);
		}
		if ((segmentoMemoria->fin - segmentoMemoria->inicio) == sizeof(tcb_t)) {
			segmentoMemoria->id = patotaId;
			segmentoMemoria->tipoDato = DATO_TCB;
			return segmentoMemoria;
		}
		uint32_t tamanioTotal = sizeof(tcb_t); // + tamanio tareas
		uint32_t fin_viejo;
		fin_viejo = segmentoMemoria->fin;
		segmentoMemoria->fin = tamanioTotal + segmentoMemoria->inicio; //YA CALCULE QUE ENTRA EN EL FIND
		segmentoMemoria->id = patotaId;
		segmentoMemoria->tipoDato = DATO_TCB;
		crear_segmento(segmentoMemoria->fin,fin_viejo);
		return segmentoMemoria;
}
segmento_t* buscar_segmentoTareas(pcb_t pcb,uint32_t tamanioTareas) {
	    //El segmento que se encontraba en memoria
		segmento_t* segmentoMemoria = segmentoTareas_segun(algoritmo,tamanioTareas);
		if(segmentoMemoria == NULL){
			pthread_mutex_lock(&accesoMemoria);
			compactar_memoria();
			pthread_mutex_unlock(&accesoMemoria);
			segmentoMemoria = segmentoTareas_segun(algoritmo,tamanioTareas);
		}
		if ((segmentoMemoria->fin - segmentoMemoria->inicio) == tamanioTareas) {
			segmentoMemoria->id = pcb.id;
			segmentoMemoria->tipoDato = DATO_TAREAS;
			return segmentoMemoria;
		}
		uint32_t tamanioTotal = tamanioTareas; // + tamanio tareas
		uint32_t fin_viejo;
		fin_viejo = segmentoMemoria->fin;
		segmentoMemoria->fin = tamanioTotal + segmentoMemoria->inicio; //YA CALCULE QUE ENTRA EN EL FIND
		segmentoMemoria->id = pcb.id;
		segmentoMemoria->tipoDato = DATO_TAREAS;
		crear_segmento(segmentoMemoria->fin,fin_viejo);
		return segmentoMemoria;
}
t_list* buscarTablaPatota(uint32_t id){
	bool condicionTablaId(void* tablaSegmentos) {
				t_list* tabla = (t_list*)tablaSegmentos;
				segmento_t* sg_pcb;
				sg_pcb = list_get(tabla,0);
				return sg_pcb->id == id;
			}
	return list_find(listaTablaSegmentos,condicionTablaId);
}
//ELIMINAR Y RECIBIR TAREAS (Creacion y borrar segmentos)

//PRIMERA APROXIMACION A CREAR PATOTA POSTA
/*int crear_patota2(pcb_t pcb,char* posiciones,char* tareas,uint32_t cantidad_trip) {
	segmento_t* segmentoAsignado;
	segmento_t* segmentoAsignadoTareas;
	uint32_t offset;
	pthread_mutex_lock(&accesoListaSegmentos);
	uint32_t espacioLibreMemoria = memoria_libre();
	uint32_t tamanioNecesario = sizeof(tcb_t)*cantidad_trip + sizeof(pcb_t) + strlen(tareas)*sizeof(char);
	pthread_mutex_unlock(&accesoListaSegmentos);
	if(espacioLibreMemoria < tamanioNecesario) {
		log_info(logger,"No hay espacio para crear la patota. Disponible: %i Necesario: %i",espacioLibreMemoria,tamanioNecesario);
		return ERROR_MEMORIA_LLENA;
	}
	pthread_mutex_lock(&accesoListaSegmentos);
	segmentoAsignado = buscar_segmento(pcb);
	if(segmentoAsignado == NULL){
		log_info(logger,"Ha ocurrido un error durante la creacion del pcb");
		return ERROR;
	}
	segmentoAsignadoTareas = buscar_segmentoTareas(pcb,tareas);
	if(segmentoAsignado == NULL){
		log_info(logger,"Ha ocurrido un error durante la creacion de las tareas");
		return ERROR;
	}
	pthread_mutex_unlock(&accesoListaSegmentos);
	pthread_mutex_lock(&accesoMemoria);
	offset = segmentoAsignado->inicio;
	memcpy(mem_ppal+offset,&pcb,sizeof(pcb_t));
	offset = segmentoAsignadoTareas->inicio;
	memcpy(mem_ppal+offset,tareas,strlen(tareas)*sizeof(char)+1);
	pthread_mutex_unlock(&accesoMemoria);
	t_list* tablaSegmentos = list_create();
	pthread_mutex_lock(&accesoListaTablas);
	list_add(tablaSegmentos,segmentoAsignado);
	list_add(tablaSegmentos,segmentoAsignadoTareas);
	list_add(listaTablaSegmentos,tablaSegmentos);
	pthread_mutex_unlock(&accesoListaTablas);
	return PATOTA_CREADA;
}*/
int crear_patota_(pcb_t pcb,char* tareas,uint32_t cantidad_tripulantes,tabla_t* tabla) {
	uint32_t tamanioTotal = sizeof(pcb_t) + sizeof(tcb_t)*cantidad_tripulantes + string_length(tareas);
	uint32_t framesNecesarios = calcular_frames(tamanioTotal);
	if(!paginacion) {
		framesNecesarios = cantidad_tripulantes + 2;
		pthread_mutex_lock(&accesoListaSegmentos);
		uint32_t espacioLibreMemoria = memoria_libre();
		pthread_mutex_unlock(&accesoListaSegmentos);
		if(espacioLibreMemoria < tamanioTotal) {
			log_error(logger,"No hay espacio para crear la patota. Disponible: %i Necesario: %i",espacioLibreMemoria,tamanioTotal);
			return ERROR_MEMORIA_LLENA;
		}
	}
	log_debug(logger,"Frames necesarios -> %i",framesNecesarios);
	tabla->idPatota = pcb.id;
	tabla->ocupado = 0;
	tabla->tamanioTareas = string_length(tareas);
	tabla->listaAsignados = list_create();
	buscar_frames(pcb.id,framesNecesarios,tabla);
	//TENGO TABLA CON PAGINAS/SEGMENTOS EMPIEZO A GUARDAR
	uint32_t i = guardarDato(tabla,(void*)&pcb,sizeof(pcb_t),tabla->ocupado);
	log_debug(logger,"Guarde %i bytes correspondientes al pcb",i);
	tabla->ocupado += i;
	tabla->tamanioTareas = string_length(tareas);
	//pcb.tareas = tablaPatota->ocupado; cuando modifique el char*
	if(!paginacion){
		tabla->ocupado = 1;
		//pcb.tareas = tablaPatota->ocupado; cuando modifique el char*
	}
	i = guardarDato(tabla,(void*)tareas,tabla->tamanioTareas,tabla->ocupado);
	log_debug(logger,"Guarde %i bytes correspondientes a tareas",i);
	tabla->ocupado += i;
	if(!paginacion){
		tabla->ocupado = 2;
	}

	//list_iterate(listaSegmentos,mostrarEstadoMemoria);
	list_add(tablasPatotaPaginacion,tabla);
	return 0;
}
int crear_tripulante_(tcb_t tcb,uint32_t idpatota,tabla_t* tablaPatota){
	uint32_t direccionLogica;
	tcb.pcb = 0;
	log_debug(logger,"Ocupado-> %i  Tareas-> %i",tablaPatota->ocupado,tablaPatota->tamanioTareas);
	if(paginacion) {
		tcb.prox_tarea = 8;
		direccionLogica = tablaPatota->ocupado;
	}
	else{
		tcb.prox_tarea = 1;
		direccionLogica = tablaPatota->ocupado;
	}
	log_debug(logger,"Guardando dato TCB DirLog: %i",direccionLogica);
	uint32_t guardado = guardarDato(tablaPatota,(void*)&tcb,sizeof(tcb_t),direccionLogica);
	log_debug(logger,"Guarde %i bytes correspondientes al tcb",guardado);
	if(paginacion){
		tablaPatota->ocupado += guardado;
	}
	else {
		tablaPatota->ocupado++;
	}


	return TRIPULANTE_CREADO;
}
//No es necesario verificar porque si no había espacio disponible nunca la llama!!
int crear_tripulante(uint32_t idTrip,uint32_t id_patota,uint32_t x,uint32_t y,uint32_t idpatota){
	segmento_t* segmentoTcb;
	t_list* tablaSegmentos;
	tcb_t tcb;
	uint32_t offset;
	tcb.id = idTrip;
	tcb.x = x;
	tcb.y = y;
	tcb.pcb = idpatota;
	tcb.prox_tarea = 0;
	pthread_mutex_lock(&accesoListaSegmentos);
	segmentoTcb = buscar_segmentoTcb(tcb,id_patota);
	pthread_mutex_unlock(&accesoListaSegmentos);
	if(segmentoTcb == NULL){
		log_info(logger,"Ha ocurrido un error durante la creacion del tripulante");
		return ERROR;
	}
	offset = segmentoTcb->inicio;
	pthread_mutex_lock(&accesoMemoria);
	memcpy(mem_ppal+offset,&tcb,sizeof(tcb_t));
	pthread_mutex_unlock(&accesoMemoria);
	pthread_mutex_lock(&accesoListaTablas);
	tablaSegmentos = buscarTablaPatota(id_patota);
	list_add(tablaSegmentos,segmentoTcb);
	pthread_mutex_unlock(&accesoListaTablas);
	return TRIPULANTE_CREADO;
}

//FIN DE FUNCIONES PARA APROXIMAR
int unificar_sg_libres(void) {
	segmento_t* anterior = NULL;
	segmento_t* actual = NULL;
	uint32_t cantidad = list_size(listaSegmentos);
	uint32_t i = 0;
	if(cantidad == 0){
		return -1;
	}
	anterior = list_get(listaSegmentos,i);
	i++;
	while (i < cantidad){
		actual = list_get(listaSegmentos,i);
		if(actual->tipoDato == VACIO && anterior ->tipoDato == VACIO) {
			anterior->fin = actual->fin;
			free(list_remove(listaSegmentos,i));
			cantidad--;
		}
		else {
		anterior = actual;
		i++;
		}
	}
	return 0;
}
void liberar_segmento(segmento_t* sg){
	sg->tipoDato = VACIO;
	sg->id = 0;

}
t_list* buscarTablaPatotaTrip(uint32_t id_trip) {
	bool buscarTabla(void* tabla){
		t_list* tablaPatota = (t_list*)tabla;
		uint32_t tamanio = list_size(tablaPatota);
		uint32_t i = 2;
		if(tamanio >= 2){
			return false;
		}
		while (tamanio > i){
			segmento_t* sg = list_get(tablaPatota,i);
			tcb_t temp;
			memcpy(&temp,mem_ppal+sg->inicio,sizeof(tcb_t));
			if (temp.id == id_trip){
				return true;
			}
			i++;
		}
		return false;
	}
	return list_find(listaTablaSegmentos,buscarTabla);
}

int eliminar_tripulante(uint32_t id_trip,uint32_t id_patota){
	pthread_mutex_lock(&accesoMemoria);
	pthread_mutex_lock(&accesoListaSegmentos);
	bool buscarsgTcb(void* seg) {
		segmento_t* sg = (segmento_t*)seg;
		if(sg->tipoDato != DATO_TCB){
			return false;
		}
		tcb_t temp;
		memcpy(&temp,mem_ppal+sg->inicio,sizeof(tcb_t));
		return temp.id == id_trip;
	}
	segmento_t* sg = list_find(listaSegmentos,buscarsgTcb);
	t_list* tablaPatota = buscarTablaPatota(id_patota);
	list_remove_by_condition(tablaPatota,buscarsgTcb);
	if(list_size(tablaPatota) <= 2) {
		eliminar_patota(id_patota);
	}
	liberar_segmento(sg);
	unificar_sg_libres();
	//t_list* tablaPatota = buscarTablaPatota(id_trip);
	pthread_mutex_unlock(&accesoMemoria);
	pthread_mutex_unlock(&accesoListaSegmentos);
	return 0;
}
int eliminar_patota(int id){
	bool listaId(void* seg){
		segmento_t* sg = (segmento_t*)seg;
		return sg->id == id;
	}
	t_list* lista = list_filter(listaSegmentos,listaId);
	liberar_segmento(list_get(lista,0));
	liberar_segmento(list_get(lista,1));
	return 0;
}
//FUNCION SIN MUTEX USARLOS ANTES DE LLAMARLA
void crear_segmento(uint32_t inicio,uint32_t fin){
	segmento_t* sg = malloc(sizeof(segmento_t));
	sg->inicio = inicio;
	sg->fin = fin;
	sg->id = 0;
	sg->tipoDato = VACIO;
	list_add_sorted(listaSegmentos,sg,ordenar_segun_inicio);
}
//FUNCIONES PARA IMPRIMIR LISTAS ITERANDO

//Funciones para mostrar por consola (Modificar a loggers!) Mutex Seteados

void mostrarMemoriaCompleta(void* segmento) {
	pthread_mutex_lock(&accesoListaSegmentos);
	pthread_mutex_lock(&accesoMemoria);
	segmento_t* sg = (segmento_t*)segmento;
	short int tipo = sg->tipoDato;
	switch (tipo) {
		case DATO_PCB: {
			pcb_t pcb;
			memcpy(&pcb,mem_ppal+sg->inicio,sizeof(pcb_t));
			log_info(logger,"Pcb-> %i",pcb.id);
			break;
		}
		case DATO_TCB: {
			tcb_t tcb;
			memcpy(&tcb,mem_ppal+sg->inicio,sizeof(tcb_t));
			log_info(logger,"Tcb-> %i X-> %i Y-> %i",tcb.id,tcb.x,tcb.y);

			break;
		}
		case DATO_TAREAS: {
			uint32_t tamanio = sg->fin - sg->inicio;
			char* str = malloc(tamanio*sizeof(char));
			memcpy(str,mem_ppal+sg->inicio,tamanio*sizeof(char));
			log_info(logger,"Tarea -> %s",str);
			free(str);
			break;
		}
		case VACIO: {
			log_info(logger,"Segmento Libre");
			break;
		}
	}
	pthread_mutex_unlock(&accesoListaSegmentos);
	pthread_mutex_unlock(&accesoMemoria);
}
void mostrarEstadoMemoria(void* segmento) {
	pthread_mutex_lock(&accesoListaSegmentos);
	segmento_t* sg = (segmento_t*)segmento;
	short int tipo = sg->tipoDato;
	uint32_t tamanio = sg->fin - sg->inicio;
	switch (tipo) {
		case DATO_PCB: {
			log_info(logger,"Pcb-Proceso: %i \t\t Inicio: %i \t Fin:%i \t Tamanio:%i \t",sg->id,sg->inicio,sg->fin,tamanio);
			break;
		}
		case DATO_TCB: {
			log_info(logger,"Tcb-Proceso: %i \t\t Inicio: %i \t Fin:%i \t Tamanio:%i \t",sg->id,sg->inicio,sg->fin,tamanio);
			break;
		}
		case DATO_TAREAS: {
			log_info(logger,"Tareas-Proceso: %i \t Inicio: %i \t Fin:%i \t Tamanio:%i \t",sg->id,sg->inicio,sg->fin,tamanio);
			break;
		}
		case VACIO: {
			log_info(logger,"Proceso: Libre \t\t Inicio: %i \t Fin:%i \t Tamanio:%i \t",sg->inicio,sg->fin,tamanio);
			break;
		}
	}
	pthread_mutex_unlock(&accesoListaSegmentos);
}

//Funciones de compactacion Las 3 funciones tienen Mutex
//OJO CON DESPLAZARSEGMENTO NO TIENE MUTEX ADENTRO USAR AFUERA!
int desplazar_segmento(segmento_t* sg,uint32_t offset) {
	if (offset == 0){
		return 0;
	}
	uint32_t tamanioTarea = (sg->fin - sg->inicio ) * sizeof(char);
	char* str = malloc(tamanioTarea);
	pcb_t pcb;
	tcb_t tcb;
	switch(sg->tipoDato){
		case DATO_PCB:

			memcpy(&pcb,mem_ppal+sg->inicio,sizeof(pcb_t));
			sg->inicio -= offset;
			sg->fin -= offset;
			memcpy(mem_ppal+sg->inicio,&pcb,sizeof(pcb_t));
			break;
		case DATO_TCB:
			memcpy(&tcb,mem_ppal+sg->inicio,sizeof(tcb_t));
			sg->inicio -= offset;
			sg->fin -= offset;
			memcpy(mem_ppal+sg->inicio,&tcb,sizeof(tcb_t));
			break;
		case DATO_TAREAS:
			memcpy(str,mem_ppal+sg->inicio,tamanioTarea);
			sg->inicio -= offset;
			sg->fin -= offset;
			memcpy(mem_ppal+sg->inicio,str,tamanioTarea);
			break;
		case VACIO:
			break;
	}
	free(str);
	return 0;
}
int memoria_libre(void) {
		segmento_t* actual = NULL;
		uint32_t cantidad = list_size(listaSegmentos);
		uint32_t i = 0;
		uint32_t libre = 0;
		if(cantidad == 0){
			return -1;
		}

		while (i < cantidad){
			actual = list_get(listaSegmentos,i);
			if(actual->tipoDato == VACIO){
				libre += actual->fin - actual->inicio;
			}
			i++;

		}
		return libre;
}
int compactar_memoria(void) {
	segmento_t* anterior = NULL;
		segmento_t* actual = NULL;
		uint32_t cantidad = list_size(listaSegmentos);
		uint32_t i = 0;
		uint32_t offset = 0;
		list_iterate(listaSegmentos,mostrarEstadoMemoria);
		log_info(logger,"--------Procediendo a compactar-------");
		if(cantidad <= 1){
			return -1;
		}
		anterior = list_get(listaSegmentos,i);
		i++;
		actual = list_get(listaSegmentos,i);
		if(anterior->tipoDato == VACIO){
			offset += anterior->fin - anterior->inicio;
			desplazar_segmento(actual,offset);
			free(list_remove(listaSegmentos,i-1));
			cantidad--;
		}
		anterior = actual;
		actual = list_get(listaSegmentos,i);
		i++;
		while (i < cantidad-1){
			if(actual->tipoDato == VACIO) {
				offset += actual->fin - actual->inicio;
				free(list_remove(listaSegmentos,i));
				cantidad--;
			}
			else {
			desplazar_segmento(actual,offset);
			i++;
			}
			anterior = actual;
			actual = list_get(listaSegmentos,i);
		}
		if(actual->tipoDato != VACIO) {
			desplazar_segmento(actual,offset);
		}
		else {
			offset += actual->fin - actual->inicio;
			free(list_remove(listaSegmentos,i));

		}
		crear_segmento(tamanio-offset,tamanio);
		return 0;
}
//Get


//PAGINACION!!
tabla_t* buscarTablaId(uint32_t id){
	bool tablaPatotaPaginacion(void* tabla) {
		tabla_t* tablaPatota = (tabla_t*)tabla;
		log_debug(logger,"Tabla encontrada %i",tablaPatota->idPatota);
		return tablaPatota->idPatota == id;
	}
	return list_find(tablasPatotaPaginacion,tablaPatotaPaginacion);
}
// CON DIRECCION LOGICA >
void* getDato(uint32_t id_patota,uint32_t tamanio,uint32_t direccionLogica){
	log_debug(logger,"Buscando tabla %i",id_patota);
	tabla_t* tabla = buscarTablaId(id_patota);
	log_debug(logger,"Buscando tabla %i",tabla->idPatota);
	log_debug(logger,"Encontre la tabla de la patota %i",tabla->idPatota);
	if(paginacion){ //SI es paginacion;
	uint32_t nPagina = direccionLogica / tamanioPagina;
	pagina_t* pagina;
	uint32_t offset = 0;
	uint32_t librePag = tamanioPagina;
	uint32_t leido = 0;
	void* aux = malloc(tamanio);
	if (direccionLogica > tamanioPagina) {
		librePag -= direccionLogica % tamanioPagina;
		offset += direccionLogica % tamanioPagina;
	}
	if (direccionLogica < tamanioPagina) {
		librePag -= direccionLogica;
		offset += direccionLogica;
	}
	if (librePag >= tamanio){
		pagina = list_get(tabla->listaAsignados,nPagina);
		offset += pagina->Nframe*tamanioPagina;
		memcpy(aux+leido,mem_ppal+offset,tamanio-leido);
		leido += tamanio-leido;
		return aux;
	}
	while(leido < tamanio && librePag < (tamanio - leido)){
		pagina = list_get(tabla->listaAsignados,nPagina);
		offset += pagina->Nframe*tamanioPagina;
		memcpy(aux+leido,mem_ppal+offset,librePag);
		leido += librePag;
		nPagina++;
		offset = 0;
		librePag = tamanioPagina;
	}
	pagina = list_get(tabla->listaAsignados,nPagina);
	offset = pagina->Nframe * tamanioPagina;
	if (leido < tamanio){
		memcpy(aux+leido,mem_ppal+offset,tamanio-leido);
		leido += tamanio-leido;
	}
	return aux;
	}
	else {
		void* aux = malloc(tamanio);
		segmento_t* segmento = list_get(tabla->listaAsignados,direccionLogica);
		log_debug(logger,"Segmento encontrado %i -> inicio %i",direccionLogica,segmento->inicio);
		memcpy(aux,mem_ppal+segmento->inicio,segmento->fin - segmento->inicio);
		return aux;
	}
}
uint32_t reconocerTamanioInstruccion(uint32_t direccionLogica,tabla_t* tabla) {
	uint32_t tamanio = 0;
	uint32_t direccionFisica = ((segmento_t*)list_get(tabla->listaAsignados,1))->inicio + direccionLogica - 1;
	if(paginacion) {
		uint32_t pagina = direccionLogica/tamanioPagina;
		uint32_t offset = 0;
		if (direccionLogica > tamanioPagina) {
			offset += direccionLogica - pagina*tamanioPagina;
		}
		if (direccionLogica < tamanioPagina) {
			offset += direccionLogica;
		}
		direccionFisica = pagina*tamanioPagina + offset;
	}
	char aux;
	while (tamanio < 100){
		memcpy(&aux,mem_ppal+direccionFisica+tamanio,sizeof(char));
		if(aux == '\0' || aux == '\n'){
			log_debug(logger,"Tamanio Proxima instruccion desde %i, es %i",direccionFisica,tamanio);
			return tamanio;
		}
		tamanio++;
	}
	return tamanio;
}
void* getInstruccion(uint32_t id_patota,uint32_t tamanio,uint32_t direccionLogica){
	log_debug(logger,"Buscando tabla %i",id_patota);
	tabla_t* tabla = buscarTablaId(id_patota);
	log_debug(logger,"Encontre la tabla de la patota %i",tabla->idPatota);
	if(paginacion){ //SI es paginacion;
	uint32_t nPagina = direccionLogica / tamanioPagina;
	pagina_t* pagina;
	uint32_t offset = 0;
	uint32_t librePag = tamanioPagina;
	uint32_t leido = 0;
	if(tabla->tamanioTareas <= (direccionLogica - sizeof(pcb_t))) {
		return "FIN";
	}
	void* aux = malloc(tamanio+1);
	if (direccionLogica > tamanioPagina) {
		librePag -= direccionLogica % tamanioPagina;
		offset += direccionLogica % tamanioPagina;
	}
	if (direccionLogica < tamanioPagina) {
		librePag -= direccionLogica;
		offset += direccionLogica;
	}
	if (librePag >= tamanio){
		pagina = list_get(tabla->listaAsignados,nPagina);
		offset += pagina->Nframe*tamanioPagina;
		memcpy(aux+leido,mem_ppal+offset,tamanio-leido);
		leido += tamanio-leido;
		*(char*)(aux+tamanio-1) = '\0';
		return aux;
	}
	while(leido < tamanio && librePag < (tamanio - leido)){
		pagina = list_get(tabla->listaAsignados,nPagina);
		offset += pagina->Nframe*tamanioPagina;
		memcpy(aux+leido,mem_ppal+offset,librePag);
		leido += librePag;
		nPagina++;
		offset = 0;
		librePag = tamanioPagina;
	}
	pagina = list_get(tabla->listaAsignados,nPagina);
	offset = pagina->Nframe * tamanioPagina;
	if (leido < tamanio){
		memcpy(aux+leido,mem_ppal+offset,tamanio-leido);
		leido += tamanio-leido;
	}
	*(char*)(aux+tamanio-1) = '\0';
	return aux;
	}
	else {
		void* aux = malloc(tamanio+1);
		segmento_t* segmento = list_get(tabla->listaAsignados,1);
		uint32_t offsetSegmento = direccionLogica-1;//SE PODRIA SACAR LA DIRECCION LOGICA INICIO EN PCB
		if(offsetSegmento >= segmento->fin - segmento->inicio -1) {
			return "FIN";
		}
		log_debug(logger,"Segmento encontrado %i, Inicio-> %i",1,segmento->inicio + offsetSegmento);
		memcpy(aux,mem_ppal+segmento->inicio+offsetSegmento,tamanio);
		*(char*)(aux+tamanio-1) = '\0';
		return aux;
	}
}
uint32_t guardarDato(tabla_t* tabla,void* dato,uint32_t tamanio,uint32_t direccionLogica) {
	if(paginacion){ //paginacion
	uint32_t nPagina = direccionLogica / tamanioPagina; //Calculo cuantas paginas debo desplazarme
		pagina_t* pagina; //Primera pagina con lugar
		uint32_t offset = 0; //Inicio del frame asociado a la pagina en MEM
		uint32_t librePag = tamanioPagina; //Asumo que esta vacia
		uint32_t guardado = 0;

		if (direccionLogica > tamanioPagina) {
			librePag -= direccionLogica % tamanioPagina;
			offset += direccionLogica % tamanioPagina;
		}
		if (direccionLogica < tamanioPagina) {
			librePag -= direccionLogica;
			offset += direccionLogica;
		}
		if (librePag >= tamanio){
			pagina = list_get(tabla->listaAsignados,nPagina);
			offset += pagina->Nframe*tamanioPagina;
			memcpy(mem_ppal+offset,dato+guardado,tamanio-guardado);
			guardado += tamanio-guardado;
			return guardado;
		}
		while(guardado < tamanio && librePag < (tamanio - guardado)){

			pagina = list_get(tabla->listaAsignados,nPagina);
			offset += pagina->Nframe*tamanioPagina;
			memcpy(mem_ppal+offset,dato+guardado,librePag);
			guardado += librePag;
			nPagina++;
			offset = 0;
			librePag = tamanioPagina;
		}
		pagina = list_get(tabla->listaAsignados,nPagina);
		offset = pagina->Nframe * tamanioPagina;
		if (guardado < tamanio){
			memcpy(mem_ppal+offset,dato+guardado,tamanio-guardado);
			guardado += tamanio-guardado;
		}
		return guardado;
	}
	else {
		segmento_t* segmento = list_get(tabla->listaAsignados,direccionLogica);
		log_debug(logger,"Guardado en tabla-> %i",tabla->idPatota);
		memcpy(mem_ppal+ segmento->inicio,dato,segmento->fin - segmento->inicio);
		return tamanio;
	}
	return 0;
}

//Mostrar Frames
void mostrarFrames(void* frame){
	frame_t* fr = (frame_t*)frame;
	if(fr->estado){
		printf("Frame: %i Estado: Libre \n",fr->numeroFrame);
	}
	else printf("Frame: %i Estado: Ocupado \n",fr->numeroFrame);
}
uint32_t calcular_frames(uint32_t tamanioTotal) {
	uint32_t i = 1;
	while (tamanioTotal >= tamanioPagina) {
		tamanioTotal -= tamanioPagina;
		i++;
	}
	return i;
}
bool condicionFrameLibre(void* valor){
	frame_t* frame = (frame_t*) valor;
	return frame->estado;
}
int buscar_frames(uint32_t id,uint32_t framesNecesarios,tabla_t* tablaPatota) {

	if(paginacion){//PAginacion
	uint32_t framesAsignados = 0;
	//BUSCAR CUANTOS FRAMES LIBRES HAY SI NO TENGO NECESARIOS INFORMAR
	while(framesAsignados < framesNecesarios) {
		frame_t* frame = list_find(framesMemoria,condicionFrameLibre);
		frame->estado = false;
		pagina_t* pagina = malloc(sizeof(pagina_t));
		pagina->Nframe = frame->numeroFrame;
		list_add(tablaPatota->listaAsignados,pagina);
		framesAsignados++;
	}
	return 0;
	}
	else{
		pcb_t pcb;
		pcb.id = id;
		segmento_t* seg = buscar_segmento(pcb);
		list_add(tablaPatota->listaAsignados,seg);
		//seg = buscar_segmentoTareas(pcb,tablaPatota->tamanioTareas);
		seg = buscar_segmentoTareas(pcb,tablaPatota->tamanioTareas) ; //MODIFICAR BNUSCAR
		list_add(tablaPatota->listaAsignados,seg);
		while ((framesNecesarios - 2) > 0 ){
			tcb_t tcb;
			seg = buscar_segmentoTcb(tcb,pcb.id);
			list_add(tablaPatota->listaAsignados,seg);
			framesNecesarios--;
		}
		return 0;
	}
}
int crear_memoria_(void){
	tablasPatotaPaginacion = list_create();
	if(paginacion){
	mem_ppal = malloc(tamanioMemoria);
	framesMemoria = list_create();
	uint32_t cantidadMarcos = tamanioMemoria/tamanioPagina;
	uint32_t i = 0;
	while(i < cantidadMarcos) {
		frame_t* frame = malloc(sizeof(frame_t));
		frame->estado = true;
		frame->numeroFrame = i;
		list_add(framesMemoria,frame);
		i++;
	}
	return 0;
	}
	else {
		mem_ppal = malloc(tamanioMemoria);
		//MEM SETT!!
		memset(mem_ppal,0,tamanioMemoria);
		listaSegmentos = list_create(); //Creo la lista de segmentos
		printf("Cree la memoria e inicialize la lista de segmentos \n");
		crear_segmento(0,tamanioMemoria); //DESCOMENTAR ESTO
		return 0;
	}
}













/// YA NO SIRVE
pcb_t getPcb (int idPedido){
	bool getPcbId(void* segmento) {
		segmento_t* segmento_tmp = (segmento_t*)segmento;
		return (segmento_tmp->id == idPedido) && segmento_tmp->tipoDato == DATO_PCB;
	}
	segmento_t* sg = list_find(listaSegmentos,getPcbId);
	pcb_t pcb_temp;
	uint32_t offset = sg->inicio;
	memcpy(&pcb_temp,mem_ppal2+offset,sizeof(pcb_t));
	return pcb_temp;
}
tcb_t getTcb (int idPedido){
	bool buscarSegmentosTcb(void* segmento) {
		segmento_t* segmento_tmp = (segmento_t*)segmento;
		return segmento_tmp->tipoDato == DATO_TCB;
	}
	bool buscarTcbId(void* segmento) {
		segmento_t* segmento_tmp = (segmento_t*)segmento;
		uint32_t offset = segmento_tmp->inicio;
		tcb_t temp;
		memcpy(&temp,mem_ppal2+offset,sizeof(tcb_t));
		return temp.id == idPedido;
	}
	t_list* lista_temporal = list_filter(listaSegmentos,buscarSegmentosTcb);
	tcb_t tcb_temp;
	segmento_t* sg = list_find(lista_temporal,buscarTcbId);
	uint32_t offset = sg->inicio;
	memcpy(&tcb_temp,mem_ppal2+offset,sizeof(tcb_t));
	return tcb_temp;
}
char* getTarea(int idPedido,uint32_t nTarea) {
	bool buscarSegmentoTareaId(void* segmento) {
		segmento_t* segmento_tmp = (segmento_t*)segmento;
		return segmento_tmp->tipoDato == DATO_TAREAS && segmento_tmp->id == idPedido;
	}
	segmento_t* sg = list_find(listaSegmentos,buscarSegmentoTareaId);
	char* tarea = malloc(sg->fin - sg->inicio);
	uint32_t offset = sg->inicio;
	memcpy(tarea,mem_ppal2+offset,sg->fin - sg->inicio);

	return reconocer_tareas(tarea,nTarea);
}

void setPcb(pcb_t pcb) {
	bool getPcbId(void* segmento) {
		segmento_t* segmento_tmp = (segmento_t*)segmento;
		return (segmento_tmp->id == pcb.id) && segmento_tmp->tipoDato == DATO_PCB;
	}
	segmento_t* sg = list_find(listaSegmentos,getPcbId);
	uint32_t offset = sg->inicio;
	memcpy(mem_ppal2+offset,&pcb,sizeof(pcb_t));
}
void setTcb (int idPedido,tcb_t tcb){
	bool buscarSegmentosTcb(void* segmento) {
		segmento_t* segmento_tmp = (segmento_t*)segmento;
		return segmento_tmp->tipoDato == DATO_TCB;
	}
	bool buscarTcbId(void* segmento) {
		segmento_t* segmento_tmp = (segmento_t*)segmento;
		uint32_t offset = segmento_tmp->inicio;
		tcb_t temp;
		memcpy(&temp,mem_ppal2+offset,sizeof(tcb_t));
		return temp.id == idPedido;
	}
	t_list* lista_temporal = list_filter(listaSegmentos,buscarSegmentosTcb);
	segmento_t* sg = list_find(lista_temporal,buscarTcbId);
	uint32_t offset = sg->inicio;
	memcpy(mem_ppal2+offset,&tcb,sizeof(tcb_t));
}
//AUXILIAR PARA TAREAS
char* reconocer_tareas(char* tarea,uint32_t tareaPedida){
	char anterior;
	char actual;
	log_info(logger,"Quiero la tarea %i",tareaPedida);
	uint32_t tamanio = strlen(tarea);
	uint32_t fin = 2;
	uint32_t nTarea = 0;
	uint32_t inicio;
	if(tarea == NULL){
		return NULL;
	}
	anterior = tarea[0];
	actual = tarea[1];
	inicio = 0;
	while (fin < tamanio){
		anterior = actual;
		actual = tarea[fin];
		if (isalpha(actual) && isdigit(anterior)){
			if(tareaPedida == nTarea) {
				char* tareaP = malloc(fin-inicio+1);
				tareaP = memset(tareaP,0,fin-inicio+1);
				memcpy(tareaP,tarea+inicio,(fin-inicio) * sizeof(char));
				tareaP[fin] = '\0';
				return tareaP;
			}
			inicio = fin;
			nTarea++;
		}
		fin++;

	}
	if (nTarea < tareaPedida){
		return "FIN";
	}
	if(nTarea == tareaPedida){
		log_debug(logger,"Ultima Tarea");
		char* tareaP = malloc(fin-inicio+1);
		tareaP = memset(tareaP,0,fin-inicio+1);
		memcpy(tareaP,tarea+inicio,(fin-inicio) * sizeof(char));
		tareaP[fin] = '\0';
		return tareaP;
	}
	char* tareaP = malloc(fin-inicio+1);
	tareaP = memset(tareaP,0,fin-inicio+1);
	memcpy(tareaP,tarea+inicio,(fin-inicio) * sizeof(char));
	tareaP[fin] = '\0';
	return tareaP;
}

int crear_memoria_ppal(uint32_t size) {
	mem_ppal2 = malloc(size);
	if(mem_ppal == NULL){
		log_info(logger,"Error al crear la memoria principal");
		return ERROR_CREACION_MEMORIA;
	}
	memset(mem_ppal2,0,tamanio);
	listaSegmentos = list_create(); //Creo la lista de segmentos
	listaTablaSegmentos = list_create();
	printf("Cree la memoria e inicialize la lista de segmentos \n");
	crear_segmento(0,tamanio);
	return OK;
}
