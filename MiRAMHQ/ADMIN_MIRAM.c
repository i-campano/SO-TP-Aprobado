/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: AFIRM
 */
#include "ADMIN_MIRAM.h"

void* mem_ppal = NULL;

int tamanio = 4096;
//PRUEBAS -->
uint32_t id_patota = 1;
uint32_t id_trip = 1;
bool algoritmo = FF;
//UTILIZADAS EN ITERACION DEL DUMP SEGMENTACION
//PARA CREAR TAREAS
//uint32_t tamanioTareas = 0;

int admin_memoria(void)
{
	printf("Inicio \n");
	pcb_t pcb2;
	pcb2.id = 4;
	pcb2.tareas = "HOla\0";
	crear_patota2(pcb2,"");
	list_iterate(listaSegmentos,mostrarEstadoMemoria);
	printf("---------------------------------- \n");
	pcb_t pcb = getPcb(1);

	printf("%s \n",pcb.tareas);
	list_iterate(listaSegmentos,mostrarEstadoMemoria);
	printf("---------------------------------- \n");
	list_iterate(listaSegmentos,mostrarMemoriaCompleta);
	printf("---------------------------------- \n");
	crear_patota(4,"Hola\0");
	unificar_sg_libres();
	list_iterate(listaSegmentos,mostrarEstadoMemoria);
	printf("---------------------------------- \n");
	list_iterate(listaSegmentos,mostrarMemoriaCompleta);
	printf("-------------COMPAC--------------------- \n");
	compactar_memoria();
	list_iterate(listaSegmentos,mostrarEstadoMemoria);
	printf("---------------------------------- \n");
	list_iterate(listaSegmentos,mostrarMemoriaCompleta);
	printf("---------------------------------- \n");
	free(mem_ppal);
	return 0;
}

void crear_memoria_ppal() {
	mem_ppal = malloc(tamanio);
	memset(mem_ppal,0,tamanio);
	listaSegmentos = list_create(); //Creo la lista de segmentos
	printf("Cree la memoria e inicialize la lista de segmentos \n");
	crear_segmento(0,tamanio);
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
segmento_t* buscar_segmentoTareas(pcb_t pcb,char* tareas) {
	    //El segmento que se encontraba en memoria
		int tamanioTareas = strlen(tareas)*sizeof(char)+1;
		segmento_t* segmentoMemoria = segmentoTareas_segun(algoritmo,tamanioTareas);
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

//ELIMINAR Y RECIBIR TAREAS (Creacion y borrar segmentos)
void crear_patota(uint32_t cant_trip,char* tareas) {
	segmento_t* segmentoAsignado;
	segmento_t* segmentoAsignadoTareas;
	uint32_t offset;
	uint32_t creados = 0;
	pcb_t pcb_tmp;
	pcb_tmp.id = id_patota;
	id_patota++;
	pcb_tmp.tareas = NULL;
	segmentoAsignado = buscar_segmento(pcb_tmp);
	printf("Agregue a las listas de segmentos \n");
	list_sort(listaSegmentos,ordenar_segun_inicio);
	segmentoAsignadoTareas = buscar_segmentoTareas(pcb_tmp,tareas);
	list_sort(listaSegmentos,ordenar_segun_inicio);
	offset = segmentoAsignado->inicio;
	memcpy(mem_ppal+offset,&pcb_tmp,sizeof(pcb_t));
	offset = segmentoAsignadoTareas->inicio;
	memcpy(mem_ppal+offset,tareas,strlen(tareas)*sizeof(char)+1);
	list_iterate(listaSegmentos,mostrarEstadoMemoria);
	printf("Agregue a las listas de segmentos \n");
	while(creados < cant_trip) {
		segmento_t* segmentoTcb;
		tcb_t tcb;
		uint32_t offset;
		tcb.id = id_trip;
		tcb.x = 10;
		tcb.y = 5;
		id_trip++;
		segmentoTcb = buscar_segmentoTcb(tcb,pcb_tmp.id);
		list_sort(listaSegmentos,ordenar_segun_inicio);
		offset = segmentoTcb->inicio;
		memcpy(mem_ppal+offset,&tcb,sizeof(tcb_t));
		creados++;
	}
}
//PRIMERA APROXIMACION A CREAR PATOTA POSTA
void crear_patota2(pcb_t pcb,char* posiciones) {
	segmento_t* segmentoAsignado;
	segmento_t* segmentoAsignadoTareas;
	uint32_t offset;
	segmentoAsignado = buscar_segmento(pcb);
	list_sort(listaSegmentos,ordenar_segun_inicio);
	segmentoAsignadoTareas = buscar_segmentoTareas(pcb,pcb.tareas);
	list_sort(listaSegmentos,ordenar_segun_inicio);
	offset = segmentoAsignado->inicio;
	memcpy(mem_ppal+offset,&pcb,sizeof(pcb_t));
	offset = segmentoAsignadoTareas->inicio;
	memcpy(mem_ppal+offset,pcb.tareas,strlen(pcb.tareas)*sizeof(char)+1);
	printf("Agregue a las listas de segmentos \n");
}
void crear_tripulante(uint32_t idTrip,uint32_t id_patota){
	segmento_t* segmentoTcb;
	tcb_t tcb;
	uint32_t offset;
	tcb.id = idTrip;
	tcb.x = 0;
	tcb.y = 0;
	segmentoTcb = buscar_segmentoTcb(tcb,id_patota);
	list_sort(listaSegmentos,ordenar_segun_inicio);
	offset = segmentoTcb->inicio;
	memcpy(mem_ppal+offset,&tcb,sizeof(tcb_t));
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
int eliminar_patota(int id){
	segmento_t* actual;
	uint32_t cantidad = list_size(listaSegmentos);
	uint32_t i = 0;
	if(cantidad == 0){
		return -1;
	}
	while(cantidad > i) {
		actual = list_get(listaSegmentos,i);
		if(actual->id == id){
			liberar_segmento(actual);
		}
		i++;
	}
	unificar_sg_libres();
	return 0;
}
void crear_segmento(uint32_t inicio,uint32_t fin){
	segmento_t* sg = malloc(sizeof(segmento_t));
	sg->inicio = inicio;
	sg->fin = fin;
	sg->id = 0;
	sg->tipoDato = VACIO;
	list_add(listaSegmentos,sg);
}
//FUNCIONES PARA IMPRIMIR LISTAS ITERANDO

//Funciones para mostrar por consola (Modificar a loggers!)

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
}

//Funciones de compactacion
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
		if(cantidad == 0){
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

pcb_t getPcb (int idPedido){
	bool getPcbId(void* segmento) {
		segmento_t* segmento_tmp = (segmento_t*)segmento;
		return (segmento_tmp->id == idPedido) && segmento_tmp->tipoDato == DATO_PCB;
	}
	segmento_t* sg = list_find(listaSegmentos,getPcbId);
	pcb_t pcb_temp;
	uint32_t offset = sg->inicio;
	memcpy(&pcb_temp,mem_ppal+offset,sizeof(pcb_t));
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
		memcpy(&temp,mem_ppal+offset,sizeof(tcb_t));
		return temp.id == idPedido;
	}
	t_list* lista_temporal = list_filter(listaSegmentos,buscarSegmentosTcb);
	tcb_t tcb_temp;
	segmento_t* sg = list_find(lista_temporal,buscarTcbId);
	uint32_t offset = sg->inicio;
	memcpy(&tcb_temp,mem_ppal+offset,sizeof(tcb_t));
	return tcb_temp;
}
