/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: AFIRM
 */
#include "ADMIN_MIRAM.h"

void* mem_ppal2 = NULL;


uint32_t tamanio;



//PRUEBAS -->
//uint32_t id_patota = 1;
//uint32_t id_trip = 1;

//UTILIZADAS EN ITERACION DEL DUMP SEGMENTACION
//PARA CREAR TAREAS
//uint32_t tamanioTareas = 0;

int admin_memoria(void)
{
	swap_fd = -1;
	paginacion = true;
	pthread_mutex_init(&accesoMemoria,NULL);
	pthread_mutex_init(&accesoListaSegmentos,NULL);
	pthread_mutex_init(&accesoListaTablas,NULL);
	crear_memoria_();
	swapFile = NULL;
	//free(mem_ppal);
	return 0;
}
//Crear Memoria
int crear_memoria_(void){
	tablasPatotaPaginacion = list_create();
	if(!strcmp(confDatos.esquema,"PAGINACION")){
	mem_ppal = malloc(tamanioMemoria);
	memset(mem_ppal,0,tamanioMemoria);
	framesMemoria = list_create();
	uint32_t cantidadMarcos = tamanioMemoria/tamanioPagina;
	uint32_t i = 0;
	if(!strcmp(confDatos.algoritmo,"LRU")){
		uint32_t tamanioSwap = confDatos.tamanioSwap;
		uint32_t paginasSwap = tamanioSwap/tamanioPagina;
		swapFrames = bitarray_create_with_mode(malloc(paginasSwap/8),paginasSwap/8,LSB_FIRST);
		while(paginasSwap > i){
			bitarray_clean_bit(swapFrames,i);//1 = LIBRE
			i++;
		}
		paginasUsadas = list_create();
	}
	i = 0;
		while(i < cantidadMarcos) {
			frame_t* frame = malloc(sizeof(frame_t));
			frame->estado = true;
			frame->numeroFrame = i;
			list_add(framesMemoria,frame);
			i++;
		}
		log_info(logger,"Cree la memoria con sus %i frames",cantidadMarcos);
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
		mem_ppal = malloc(tamanioMemoria);
		//MEM SETT!!
		memset(mem_ppal,0,tamanioMemoria);
		listaSegmentos = list_create(); //Creo la lista de segmentos
		printf("Cree la memoria e inicialize la lista de segmentos \n");
		crear_segmento(0,tamanioMemoria); //DESCOMENTAR ESTO

	}
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
bool condicionFrameLibre(void* valor){
	frame_t* frame = (frame_t*) valor;
	return frame->estado;
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
			compactar_memoria();
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
			compactar_memoria();
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
			compactar_memoria();
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
tabla_t* buscarTablaId(uint32_t id){
	bool tablaPatotaPaginacion(void* tabla) {
		tabla_t* tablaPatota = (tabla_t*)tabla;
		return tablaPatota->idPatota == id;
	}
	return list_find(tablasPatotaPaginacion,tablaPatotaPaginacion);
}
//ELIMINAR Y RECIBIR TAREAS (Creacion y borrar segmentos)

int crear_patota_(pcb_t pcb,char* tareas,uint32_t cantidad_tripulantes,tabla_t* tabla) {
	uint32_t tamanioTotal = sizeof(pcb_t) + sizeof(tcb_t)*cantidad_tripulantes + string_length(tareas);
	uint32_t framesNecesarios = calcular_frames(tamanioTotal);
	if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
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
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		tabla->ocupado = 1;
		//pcb.tareas = tablaPatota->ocupado; cuando modifique el char*
	}
	i = guardarDato(tabla,(void*)tareas,tabla->tamanioTareas,tabla->ocupado);
	log_debug(logger,"Guarde %i bytes correspondientes a tareas",i);
	tabla->ocupado += i;
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		tabla->ocupado = 2;
	}

	//list_iterate(listaSegmentos,mostrarEstadoMemoria);
	list_add(tablasPatotaPaginacion,tabla);
	return PATOTA_CREADA;
}
int crear_tripulante_(tcb_t tcb,uint32_t idpatota,tabla_t* tablaPatota){
	uint32_t direccionLogica;
	tcb.pcb = 0;
	log_debug(logger,"Ocupado-> %i  Tareas-> %i",tablaPatota->ocupado,tablaPatota->tamanioTareas);
	if(!strcmp(confDatos.esquema,"PAGINACION")) {
		tcb.prox_tarea = 8;
		direccionLogica = tablaPatota->ocupado;
		log_debug(logger,"Dir Log: %i Trip: %i",direccionLogica,tcb.id);
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		tcb.prox_tarea = 1;
		direccionLogica = tablaPatota->ocupado;
	}
	log_debug(logger,"Guardando dato TCB DirLog: %i",direccionLogica);
	uint32_t guardado = guardarDato(tablaPatota,(void*)&tcb,sizeof(tcb_t),direccionLogica);
	log_debug(logger,"Guarde %i bytes correspondientes al tcb",guardado);
	if(!strcmp(confDatos.esquema,"PAGINACION")){
		tablaPatota->ocupado += guardado;
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
		tablaPatota->ocupado++;
	}


	return TRIPULANTE_CREADO;
}

//SEGMENTACION
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
void crear_segmento(uint32_t inicio,uint32_t fin){
	segmento_t* sg = malloc(sizeof(segmento_t));
	sg->inicio = inicio;
	sg->fin = fin;
	sg->id = 0;
	sg->tipoDato = VACIO;
	list_add_sorted(listaSegmentos,sg,ordenar_segun_inicio);
}
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
int eliminar_patota(tabla_t* tabla){
	uint32_t id = tabla->idPatota;
	bool tablaPatotaPaginacion(void* tabla) {
		tabla_t* tablaPatota = (tabla_t*)tabla;
		return tablaPatota->idPatota == id;
	}
	if(!strcmp(confDatos.esquema,"PAGINACION")){
		int paginas = list_size(tabla->listaAsignados);
		paginas--;
		while(paginas >= 0){
			pagina_t* paginaAsignada = list_remove(tabla->listaAsignados,paginas);
			frame_t* frame = list_get(framesMemoria,paginaAsignada->Nframe);
			frame->estado = true;
			free(paginaAsignada);
			paginas--;
		}
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
		int cantidad = list_size(tabla->listaAsignados);
		cantidad--;
		while(cantidad >= 0) {
			liberar_segmento(list_remove(tabla->listaAsignados,cantidad));
			cantidad--;
		}
	}
	free(list_remove_by_condition(tablasPatotaPaginacion,tablaPatotaPaginacion));
	return 0;
}
int eliminar_tripulante(tabla_t* tabla,uint32_t direccionLogica){
	uint32_t liberado = liberar_bytes(tabla,direccionLogica,sizeof(tcb_t));
	log_debug(logger,"Se liberaron %i bytes correspondientes a tcb",liberado);
	if(!strcmp(confDatos.esquema,"PAGINACION")){
		if(tabla->ocupado == (tabla->tamanioTareas + sizeof(pcb_t))){
			log_debug(logger,"Eliminando patota no hay mas tripulantes activos");
			eliminar_patota(tabla);
		}
		list_iterate(framesMemoria,mostrarFrames);
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
		if(tabla->ocupado == 2){
			log_debug(logger,"Eliminando patota no hay mas tripulantes activos");
			eliminar_patota(tabla);
		}
		list_iterate(listaSegmentos,mostrarEstadoMemoria);
	}
	return liberado;
}
int liberar_bytes(tabla_t* tabla,uint32_t direccionLogica,uint32_t tamanio) {
	if(!strcmp(confDatos.esquema,"PAGINACION")){
		uint32_t nPagina = direccionLogica/tamanioPagina;
		uint32_t offset = 0;
		uint32_t liberado = 0;
		if (direccionLogica > tamanioPagina) {
			offset += direccionLogica - nPagina*tamanioPagina;
		}
		if (direccionLogica < tamanioPagina) {
			offset += direccionLogica;
		}
		if(tamanio < tamanioPagina - offset){
			pagina_t* pag = list_get(tabla->listaAsignados,nPagina);
			if(!pag->valida){
				tabla->ocupado -= tamanio;
				pag->bytesOcupado -= tamanio;
				if(pag->bytesOcupado == 0){
					bitarray_set_bit(swapFrames,pag->NframeVirtual);
				}
			}
			else{
			pag->bytesOcupado -= tamanio;
			if(pag->bytesOcupado == 0){
				frame_t* fr = list_get(framesMemoria,pag->Nframe);
				fr->estado = true;
			}
			tabla->ocupado -= tamanio;
			}
			return tamanio;
		}
		pagina_t* pagina;
		while(tamanio > liberado && (tamanioPagina-offset) <= (tamanio-liberado)){
			pagina = list_get(tabla->listaAsignados,nPagina);
			if(!pagina->valida){
				pagina->bytesOcupado -= tamanioPagina-offset;
				liberado += tamanioPagina-offset;
				if(pagina->bytesOcupado == 0){
					bitarray_set_bit(swapFrames,pagina->NframeVirtual);
				}
			}
			else{
			pagina->bytesOcupado -= tamanioPagina-offset;
			liberado += tamanioPagina-offset;
			log_debug(logger,"Liberando %i bytes, pagina Ocupado-> %i",tamanioPagina-offset,pagina->bytesOcupado);
			if(pagina->bytesOcupado == 0){
				frame_t* fr = list_get(framesMemoria,pagina->Nframe);
				fr->estado = true;
			}
			}
			nPagina++;
			pagina = list_get(tabla->listaAsignados,nPagina);
			offset = 0;
		}
		if(tamanio > liberado){
			if(!pagina->valida){
				pagina->bytesOcupado -= tamanio-liberado;
				if(pagina->bytesOcupado == 0){
					bitarray_set_bit(swapFrames,pagina->NframeVirtual);
				}
				log_debug(logger,"Liberando %i bytes, paginaVIRTUAL Ocupado-> %i",tamanio - liberado,pagina->bytesOcupado);
				liberado += tamanio - liberado;
			}
			else{
			pagina->bytesOcupado -= tamanio-liberado;
			log_debug(logger,"Liberando %i bytes, pagina Ocupado-> %i",tamanio - liberado,pagina->bytesOcupado);
			if(pagina->bytesOcupado == 0){
				frame_t* fr = list_get(framesMemoria,pagina->Nframe);
				fr->estado = true;
			}
			liberado += tamanio - liberado;
			}
		}
		tabla->ocupado -= liberado;
		return liberado;
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		liberar_segmento(list_get(tabla->listaAsignados,direccionLogica));
		tabla->ocupado -= 1;
		return 1;
	}
	return 0;
}

//Funciones para mostrar por consola (Modificar a loggers!) Mutex Seteados

void mostrarMemoriaCompleta(void* segmento) {
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
//Mostrar Frames
void mostrarFrames(void* frame){
	frame_t* fr = (frame_t*)frame;
	if(fr->estado){
		log_info(logger,"Frame: %i Estado: Libre \n",fr->numeroFrame);
	}
	else log_info(logger,"Frame: %i Estado: Ocupado \n",fr->numeroFrame);
}

// CON DIRECCION LOGICA >
/*
uint32_t reconocerTamanioInstruccion(uint32_t direccionLogica,tabla_t* tabla) {
	uint32_t tamanio = 0;
	uint32_t direccionFisica;
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		direccionFisica = ((segmento_t*)list_get(tabla->listaAsignados,1))->inicio + direccionLogica - 1;
	}
	if(!strcmp(confDatos.esquema,"PAGINACION")) {
		uint32_t pagina = direccionLogica/tamanioPagina;
		uint32_t offset = 0;
		if (direccionLogica > tamanioPagina) {
			offset += direccionLogica - pagina*tamanioPagina;
		}
		if (direccionLogica < tamanioPagina) {
			offset += direccionLogica;
		}
		direccionFisica = ((pagina_t*)list_get(tabla->listaAsignados,pagina))->Nframe*tamanioPagina + offset;
		log_debug(logger,"Buscando instruccion desde %i logica",direccionFisica);
	}
	char aux;
	log_debug(logger,"Buscando instruccion desde %i logica",direccionLogica);
	while (tamanio < 100){
		memcpy(&aux,mem_ppal+direccionFisica+tamanio,sizeof(char));
		if(aux == '\0' || aux == '\n'){
			log_debug(logger,"Tamanio Proxima instruccion desde %i, es %i",direccionFisica,tamanio);
			if(aux == '\0'){
				return tamanio-1;
			}
			return tamanio;
		}
		tamanio++;
	}
	return tamanio;
}*/
uint32_t paginaTareas(int tamanioTarea){
	uint32_t paginas = 0;
	if(sizeof(pcb_t) % tamanioPagina){
		tamanioTarea += sizeof(pcb_t) % tamanioPagina;
	}
	while(tamanioTarea > 0){
		tamanioTarea -= tamanioPagina;
		paginas++;
	}
	return paginas;
}
uint32_t reconocerTamanioInstruccion(uint32_t direccionLogica,tabla_t* tabla) {
	uint32_t tamanio = 0;
	uint32_t direccionFisica;
	uint32_t i = 0;
	uint32_t paginasOcupadasTareas = paginaTareas(tabla->tamanioTareas);
	while(i < paginasOcupadasTareas){
		pagina_t* pagina = list_get(tabla->listaAsignados,i);
		if(!pagina->valida){
			realizarSwap(pagina);
		}
		i++;
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		direccionFisica = ((segmento_t*)list_get(tabla->listaAsignados,1))->inicio + direccionLogica - 1;
	}
	if(!strcmp(confDatos.esquema,"PAGINACION")) {
		uint32_t pagina = direccionLogica/tamanioPagina;
		uint32_t offset = 0;
		if (direccionLogica > tamanioPagina) {
			offset += direccionLogica - pagina*tamanioPagina;
		}
		if (direccionLogica < tamanioPagina) {
			offset += direccionLogica;
		}
		direccionFisica = ((pagina_t*)list_get(tabla->listaAsignados,pagina))->Nframe*tamanioPagina + offset;
		log_debug(logger,"Buscando instruccion desde %i logica",direccionFisica);
	}
	char aux;
	log_debug(logger,"Buscando instruccion desde %i logica",direccionLogica);
	while (tamanio < 100){
		memcpy(&aux,mem_ppal+direccionFisica+tamanio,sizeof(char));
		if(aux == '\0' || aux == '\n'){
			log_debug(logger,"Tamanio Proxima instruccion desde %i, es %i",direccionFisica,tamanio);
			if(aux == '\0'){
				return tamanio-1;
			}
			return tamanio;
		}
		tamanio++;
	}
	return tamanio;
}
uint32_t reconocerTamanioInstruccion3(uint32_t direccionLogica,tabla_t* tabla) {
	uint32_t tamanio = 0;
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		uint32_t direccionFisica;
		direccionFisica = ((segmento_t*)list_get(tabla->listaAsignados,1))->inicio + direccionLogica - 1;
		char aux;
			log_debug(logger,"Buscando instruccion desde %i fisica",direccionLogica);
			while (tamanio < 100){
				memcpy(&aux,mem_ppal+direccionFisica+tamanio,sizeof(char));
				if(aux == '\0' || aux == '\n'){
					log_debug(logger,"Tamanio Proxima instruccion desde %i, es %i",direccionFisica,tamanio);
					if(aux == '\0'){
						return tamanio-1;
					}
					return tamanio;
				}
				tamanio++;
			}
			return tamanio;
	}
	if(!strcmp(confDatos.esquema,"PAGINACION")) {
		uint32_t nPagina = direccionLogica/tamanioPagina;
		uint32_t offset = 0;
		pagina_t* pagina = list_get(tabla->listaAsignados,nPagina);
		if(!pagina->valida){
			realizarSwap(pagina);
		}
		else{
			actualizarPagina(pagina);
		}
		if (direccionLogica > tamanioPagina) {
				offset += direccionLogica % tamanioPagina;
		}
		if (direccionLogica < tamanioPagina) {
				offset += direccionLogica;
		}

		uint32_t offsetMem =pagina->Nframe*tamanioPagina;
		char aux;
		uint32_t maxPag = list_size(tabla->listaAsignados);
		while(nPagina < maxPag){
			memcpy(&aux,mem_ppal+offset+offsetMem,sizeof(char));
			log_debug(logger,"%c",aux);
				if(aux == '\0' || aux == '\n'){
					log_debug(logger,"EL tamanio de la tarea es %i",tamanio);
					if(aux == '\0'){
						return tamanio-1;
					}
					return tamanio;
				}
			if(offset == tamanioPagina-1) {
				nPagina++;
				pagina = list_get(tabla->listaAsignados,nPagina);
				if(!pagina->valida){
							realizarSwap(pagina);
				}
				else{
					actualizarPagina(pagina);
				}
				offsetMem = pagina->Nframe*tamanioPagina;
				offset = 0;
			}
			else{
				offset++;
			}
			tamanio++;
		}

	}
	return 0;
}
uint32_t reconocerTamanioInstruccion2(uint32_t direccionLogica,tabla_t* tabla) {
	uint32_t tamanio = 0;
	uint32_t direccionFisica;

	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		direccionFisica = ((segmento_t*)list_get(tabla->listaAsignados,1))->inicio + direccionLogica - 1;
	}
	if(!strcmp(confDatos.esquema,"PAGINACION")) {
		uint32_t pagina = direccionLogica/tamanioPagina;
		uint32_t offset = 0;
		if (direccionLogica > tamanioPagina) {
			offset += direccionLogica - pagina*tamanioPagina;
		}
		if (direccionLogica < tamanioPagina) {
			offset += direccionLogica;
		}
		pagina_t* paginaP = list_get(tabla->listaAsignados,pagina);
		if(!paginaP->valida){
			realizarSwap(paginaP);
		}
		offset += paginaP->Nframe*tamanioPagina;
		log_debug(logger,"Buscando instruccion desde %i logica Offset: %i",direccionLogica,offset);
		char aux = 'c';
		while (tamanio < 100){
			memcpy(&aux,mem_ppal+offset+tamanio,sizeof(char));
			log_debug(logger,"Encontre -> %c",aux);
			if(aux == '\0' || aux == '\n'){
				log_debug(logger,"Tamanio Proxima instruccion desde %i, es %i",direccionLogica,tamanio);
				if(aux == '\0'){
					return tamanio-1;
				}
				return tamanio;
			}
			offset++;
			if(offset % tamanioPagina){
				pagina++;
				paginaP = list_get(tabla->listaAsignados,pagina);
				if(!paginaP->valida){
					realizarSwap(paginaP);
				}
				offset = paginaP->Nframe*tamanioPagina;
			}
			tamanio++;
		}
	}
	char aux;
	log_debug(logger,"Buscando instruccion desde %i logica",direccionLogica);
	while (tamanio < 100){
		memcpy(&aux,mem_ppal+direccionFisica+tamanio,sizeof(char));
		if(aux == '\0' || aux == '\n'){
			log_debug(logger,"Tamanio Proxima instruccion desde %i, es %i",direccionFisica,tamanio);
			if(aux == '\0'){
				return tamanio-1;
			}
			return tamanio;
		}
		tamanio++;
	}
	return tamanio;
}
void* getDato(uint32_t id_patota,uint32_t tamanio,uint32_t direccionLogica){
	log_debug(logger,"Buscando tabla %i",id_patota);
	tabla_t* tabla = buscarTablaId(id_patota);
	if(!strcmp(confDatos.esquema,"PAGINACION")){ //SI es paginacion;
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
		if(!pagina->valida){
			realizarSwap(pagina);
		}
		else{
			actualizarPagina(pagina);
		}
		offset += pagina->Nframe*tamanioPagina;
		memcpy(aux+leido,mem_ppal+offset,tamanio-leido);
		leido += tamanio-leido;
		return aux;
	}
	while(leido < tamanio && librePag < (tamanio - leido)){
		pagina = list_get(tabla->listaAsignados,nPagina);
		if(!pagina->valida){
			realizarSwap(pagina);
		}
		else{
			actualizarPagina(pagina);
		}
		offset += pagina->Nframe*tamanioPagina;
		memcpy(aux+leido,mem_ppal+offset,librePag);
		leido += librePag;
		nPagina++;
		offset = 0;
		librePag = tamanioPagina;
	}
	pagina = list_get(tabla->listaAsignados,nPagina);
	if(!pagina->valida){
		realizarSwap(pagina);
	}
	else{
		actualizarPagina(pagina);
	}
	offset = pagina->Nframe * tamanioPagina;
	if (leido < tamanio){
		memcpy(aux+leido,mem_ppal+offset,tamanio-leido);
		leido += tamanio-leido;
	}
	return aux;
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
		void* aux = malloc(tamanio);
		segmento_t* segmento = list_get(tabla->listaAsignados,direccionLogica);
		log_debug(logger,"Segmento encontrado %i -> inicio %i",direccionLogica,segmento->inicio);
		memcpy(aux,mem_ppal+segmento->inicio,segmento->fin - segmento->inicio);
		return aux;
	}
	return NULL;
}
void* getInstruccion(uint32_t id_patota,uint32_t tamanio,uint32_t direccionLogica){
	tabla_t* tabla = buscarTablaId(id_patota);
	if(!strcmp(confDatos.esquema,"PAGINACION")){ //SI es paginacion;
	uint32_t nPagina = direccionLogica / tamanioPagina;
	pagina_t* pagina;
	uint32_t offset = 0;
	uint32_t librePag = tamanioPagina;
	uint32_t leido = 0;
	if(tabla->tamanioTareas <= (direccionLogica - sizeof(pcb_t))) {
		char* tarea = string_new();
		string_append(&tarea,"FIN\0");
		return tarea;
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
		if(!pagina->valida){
			realizarSwap(pagina);
		}
		else{
			actualizarPagina(pagina);
		}
		offset += pagina->Nframe*tamanioPagina;
		memcpy(aux+leido,mem_ppal+offset,tamanio-leido);
		leido += tamanio-leido;
		*(char*)(aux+tamanio) = '\0';
		return aux;
	}
	while(leido < tamanio && librePag < (tamanio - leido)){
		pagina = list_get(tabla->listaAsignados,nPagina);
		if(!pagina->valida){
			realizarSwap(pagina);
		}
		else{
			actualizarPagina(pagina);
		}
		offset += pagina->Nframe*tamanioPagina;
		memcpy(aux+leido,mem_ppal+offset,librePag);
		leido += librePag;
		nPagina++;
		offset = 0;
		librePag = tamanioPagina;
	}
	pagina = list_get(tabla->listaAsignados,nPagina);
	if(!pagina->valida){
		realizarSwap(pagina);
	}
	else{
		actualizarPagina(pagina);
	}
	offset = pagina->Nframe * tamanioPagina;
	if (leido < tamanio){
		memcpy(aux+leido,mem_ppal+offset,tamanio-leido);
		leido += tamanio-leido;
	}
	*(char*)(aux+tamanio) = '\0';
	return aux;
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
		void* aux = malloc(tamanio+1);
		segmento_t* segmento = list_get(tabla->listaAsignados,1);
		uint32_t offsetSegmento = direccionLogica-1;//SE PODRIA SACAR LA DIRECCION LOGICA INICIO EN PCB
		if(offsetSegmento >= segmento->fin - segmento->inicio -1) {
			char* tarea = string_new();
			string_append(&tarea,"FIN\0");
			return "FIN";
		}
		log_debug(logger,"Segmento encontrado %i, Inicio-> %i",1,segmento->inicio + offsetSegmento);
		memcpy(aux,mem_ppal+segmento->inicio+offsetSegmento,tamanio);
		*(char*)(aux+tamanio) = '\0';
		return aux;
	}
	return NULL;
}
//Estoy seguro que siempre que se llama esta funcion las paginas son validas pero por seguridad
//Agregue logica
uint32_t guardarDato(tabla_t* tabla,void* dato,uint32_t tamanio,uint32_t direccionLogica) {
	if(!strcmp(confDatos.esquema,"PAGINACION")){ //paginacion
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
			if(!pagina->valida){
				realizarSwap(pagina);
			}
			offset += pagina->Nframe*tamanioPagina;
			memcpy(mem_ppal+offset,dato+guardado,tamanio-guardado);
			pagina->bytesOcupado += tamanio;
			log_debug(logger,"Bytes ocupados %i en pagina %i",pagina->bytesOcupado,nPagina);
			guardado += tamanio-guardado;
			return guardado;
		}
		while(guardado < tamanio && librePag < (tamanio - guardado)){

			pagina = list_get(tabla->listaAsignados,nPagina);
			if(!pagina->valida){
				realizarSwap(pagina);
			}
			offset += pagina->Nframe*tamanioPagina;
			memcpy(mem_ppal+offset,dato+guardado,librePag);
			guardado += librePag;
			pagina->bytesOcupado += librePag;
			log_debug(logger,"Bytes ocupados %i en pagina %i",pagina->bytesOcupado,nPagina);
			nPagina++;
			offset = 0;
			librePag = tamanioPagina;
		}
		pagina = list_get(tabla->listaAsignados,nPagina);
		if(!pagina->valida){
			realizarSwap(pagina);
		}
		offset = pagina->Nframe * tamanioPagina;
		if (guardado < tamanio){
			memcpy(mem_ppal+offset,dato+guardado,tamanio-guardado);
			pagina->bytesOcupado += tamanio-guardado;
			log_debug(logger,"Bytes ocupados %i en pagina %i",pagina->bytesOcupado,nPagina);
			guardado += tamanio-guardado;
		}
		return guardado;
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
		segmento_t* segmento = list_get(tabla->listaAsignados,direccionLogica);
		memcpy(mem_ppal+ segmento->inicio,dato,segmento->fin - segmento->inicio);
		return tamanio;
	}
	return 0;
}
uint32_t actualizarDato(tabla_t* tabla,void* dato,uint32_t tamanio,uint32_t direccionLogica) {
	if(!strcmp(confDatos.esquema,"PAGINACION")){ //paginacion
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
			if(!pagina->valida){
				realizarSwap(pagina);
			}
			else{
				actualizarPagina(pagina);
			}
			offset += pagina->Nframe*tamanioPagina;
			memcpy(mem_ppal+offset,dato+guardado,tamanio-guardado);
			guardado += tamanio-guardado;
			return guardado;
		}
		while(guardado < tamanio && librePag < (tamanio - guardado)){

			pagina = list_get(tabla->listaAsignados,nPagina);
			if(!pagina->valida){
				realizarSwap(pagina);
			}
			else{
				actualizarPagina(pagina);
			}
			offset += pagina->Nframe*tamanioPagina;
			memcpy(mem_ppal+offset,dato+guardado,librePag);
			guardado += librePag;
			nPagina++;
			offset = 0;
			librePag = tamanioPagina;
		}
		pagina = list_get(tabla->listaAsignados,nPagina);
		if(!pagina->valida){
			realizarSwap(pagina);
		}
		else{
			actualizarPagina(pagina);
		}
		offset = pagina->Nframe * tamanioPagina;
		if (guardado < tamanio){
			memcpy(mem_ppal+offset,dato+guardado,tamanio-guardado);
			guardado += tamanio-guardado;
		}
		return guardado;
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")) {
		segmento_t* segmento = list_get(tabla->listaAsignados,direccionLogica);
		memcpy(mem_ppal+ segmento->inicio,dato,segmento->fin - segmento->inicio);
		return tamanio;
	}
	return 0;
}

uint32_t calcular_frames(uint32_t tamanioTotal) {
	uint32_t i = 1;
	while (tamanioTotal >= tamanioPagina) {
		tamanioTotal -= tamanioPagina;
		i++;
	}
	return i;
}
int llevarNframesSwap(uint32_t n){
	while(n > 0){
		uint32_t frameLibre;
		pagina_t* pagina = paginaSegun(confDatos.algoritmo);
		llevarPaginaASwap(pagina,&frameLibre);
		log_debug(logger,"Frame Liberado -> %i",frameLibre);
		n--;
	}
	if(n != 0){
		log_error(logger,"Hubo un error al liberar los frames necesarios");
		return ERROR;
	}
	return OK;
}
int buscar_frames(uint32_t id,uint32_t framesNecesarios,tabla_t* tablaPatota) {

	if(!strcmp(confDatos.esquema,"PAGINACION")){//PAginacion
	uint32_t framesAsignados = 0;
	uint32_t framesLibresMemoria = calcularFramesLibres();
	if(framesLibresMemoria < framesNecesarios){
		if(!swapFile){
			inicializarAreaSwap();
		}
		llevarNframesSwap(framesNecesarios - framesLibresMemoria);
	}
	while(framesAsignados < framesNecesarios) {
		frame_t* frame = list_find(framesMemoria,condicionFrameLibre);
		uint32_t frameSwap = frameLibreSwap();
		frame->estado = false;
		pagina_t* pagina = malloc(sizeof(pagina_t));
		pagina->Nframe = frame->numeroFrame;
		pagina->bytesOcupado = 0;
		pagina->NframeVirtual = frameSwap;
		pagina->valida = true;
		log_debug(logger,"Pagina Creada Frame: %i FrameVirtual: %i",pagina->Nframe,pagina->NframeVirtual);
		list_add(tablaPatota->listaAsignados,pagina);
		list_add(paginasUsadas,pagina);
		framesAsignados++;
	}
	return 0;
	}
	if(!strcmp(confDatos.esquema,"SEGMENTACION")){
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
	}
	return 0;
}


int inicializarAreaSwap(void){
	swapFile = fopen(confDatos.pathSwap,"w");
	fclose(swapFile);
	swapFile = fopen(confDatos.pathSwap,"rb+");
	char* aux = malloc(confDatos.tamanioSwap);
	memset(aux,0,confDatos.tamanioSwap);
	if(!swapFile){
		log_error(logger,"No se pudo abrir el archivo Swap");
		return ERROR;
	}
	fwrite(aux,1,confDatos.tamanioSwap,swapFile);
	free(aux);
	return 0;
}
int frameLibreSwap(void){
	uint32_t i = 0;
	uint32_t tamanioSwap = confDatos.tamanioSwap;
	while (i < tamanioSwap/tamanioPagina){
		if(!bitarray_test_bit(swapFrames,i)){
			bitarray_set_bit(swapFrames,i);
			return i;
		}
		i++;
	}
	return tamanioSwap/tamanioPagina + 1;
}
int llevarPaginaASwap(pagina_t* paginaASwap,uint32_t* frameLiberado){
	uint32_t offset = paginaASwap->NframeVirtual*tamanioPagina;
	uint32_t offsetMem = paginaASwap->Nframe * tamanioPagina;
	frame_t* frame = list_get(framesMemoria,paginaASwap->Nframe);
	char* aux = malloc(tamanioPagina);
	memcpy(aux,mem_ppal+offsetMem,tamanioPagina);
	fseek(swapFile,offset,SEEK_SET);
	fwrite(aux,tamanioPagina,sizeof(char),swapFile);
	frame->estado = true;
	*frameLiberado = paginaASwap->Nframe;
	log_debug(logger,"Frame liberado: %i",*frameLiberado);
	paginaASwap->valida = false;
	free(aux);
	return SWAP_OK;
}
int traerPaginaMemoria(pagina_t* pagina,uint32_t offsetMemoria) {

	uint32_t offsetSwap = pagina->NframeVirtual*tamanioPagina;
	void* aux = malloc(tamanioPagina);
	fseek(swapFile,offsetSwap,SEEK_SET);
	fread(aux,1,tamanioPagina,swapFile);
	log_debug(logger,"Copiando en offset %i",offsetMemoria);
	memcpy(mem_ppal+offsetMemoria,aux,tamanioPagina);
	pagina->Nframe = offsetMemoria/tamanioPagina;
	pagina->valida = true;
	frame_t* frame = list_get(framesMemoria,pagina->Nframe);
	log_debug(logger,"Frame utilizado para swapear: %i",offsetMemoria/tamanioPagina);
	frame->estado = false;
	list_add(paginasUsadas,pagina);
	return SWAP_OK;
}
int calcularFramesLibres(void) {
	uint32_t framesLibres = 0;
	uint32_t i = 0;
	frame_t* frame;
	while(i < (tamanioMemoria/tamanioPagina)){
		frame = list_get(framesMemoria,i);
		if(frame->estado){
			framesLibres++;
		}
		i++;
	}
	return framesLibres;
}
//SIRVE PARA TRAER PAGINA A MEMORIA SI EXISTE
int realizarSwap(pagina_t* paginaSwap){
	if(!calcularFramesLibres()){
		log_debug(logger,"No existen frames libres en memoria, procediendo a intercambiar 2 paginas");
		pagina_t* paginaEncontrada = paginaSegun(confDatos.algoritmo);
		uint32_t frameLiberado;
		llevarPaginaASwap(paginaEncontrada,&frameLiberado);

		traerPaginaMemoria(paginaSwap,frameLiberado*tamanioPagina);
	}
	else{
		frame_t* frame = list_find(framesMemoria,condicionFrameLibre);
		log_debug(logger,"Se encontro un frame Libre en memoria, procediendo a traer la pagina pedida");
		traerPaginaMemoria(paginaSwap,frame->numeroFrame * tamanioPagina);
	}
	return 0;
}
//PARA LRU

int actualizarPagina(pagina_t* paginaBuscada){
	bool quitarPagina(void* dato){
		pagina_t* pagina = (pagina_t*)dato;
		return pagina->Nframe == paginaBuscada->Nframe;
	}
	log_debug(logger,"Actualizando pagina LRU");
	list_add(paginasUsadas,list_remove_by_condition(paginasUsadas,quitarPagina));
	return 0;
}
pagina_t* paginaSegun(char* algoritmoRemplazo){
	if(!strcmp(algoritmoRemplazo,"LRU")){
	return (pagina_t*)list_remove(paginasUsadas,0);
	}
	return NULL;
}
