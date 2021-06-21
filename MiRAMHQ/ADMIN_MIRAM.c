/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: AFIRM
 */
#include "ADMIN_MIRAM.h"

void* mem_ppal = NULL;
t_list* listaSegmentos;
t_list* tablaSegmentosPorPatota;
int tamanio = 4096;
//PRUEBAS -->
uint32_t id_patota = 1;
uint32_t id_trip = 1;
t_list tablaSegmento;

//UTILIZADAS EN ITERACION DEL DUMP SEGMENTACION
uint32_t nSegmento = 1; //UTILZADA EN LISTA DE SEGMENTOS POR PATOTA
uint32_t nProceso = 0;
//PARA CREAR TAREAS
//uint32_t tamanioTareas = 0;

int admin_miram(void)
{
	printf("Inicio \n");
	crear_memoria_ppal();
	//--------------------------
	printf("Voy a crear patotas \n");
	crear_patota(1,"1234567890\0");
	printf("--------------------------------------------------------- \n");
	//--------------------------
	crear_patota(1,"GENERAR_OXIGENO 0;1;1;1GENERAR_OXIGENO 0;1;1;1\0");
	printf("--------------------------------------------------------- \n");
	//--------------------------
	crear_patota(6,"GENERAR_OXIGENO 0;1;1;1GENERAR_OXIGENO 0;1;1;1\0");
	eliminar_patota(2);
	crear_patota(1,"1\0");
	crear_patota(3,"Gernrearajsdkajsdlkasdasd\0");
	crear_patota(3,"Gernrearajsdkajsdlkasdasd\0");
	eliminar_patota(1);
	eliminar_patota(4);
	crear_patota(3,"Gernrearajsdkajsdlkasdasd\0");
	list_iterate(listaSegmentos,mostrarEstadoMemoria);
	printf("---------------------------------- \n");
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
	//MEM SETT!!
	memset(mem_ppal,0,tamanio);
	listaSegmentos = list_create(); //Creo la lista de segmentos
	tablaSegmentosPorPatota = list_create();//Con filter vuela
	printf("Cree la memoria e inicialize la lista de segmentos \n");
	segmento_t* segmento = malloc(sizeof(segmento_t));
	segmento->inicio = 0;
	segmento->fin = tamanio;
	segmento->id = 0; //0 = VACIO
	segmento->tipoDato = VACIO;
	list_add(listaSegmentos,segmento);

}

//Criterios para listas de segmentos
bool condicionSegmentoLibrePcb(void* segmento) {
	segmento_t* segmento_tmp = (segmento_t*)segmento;
	uint32_t tamanioTotal = sizeof(pcb_t);
	return (segmento_tmp->fin - segmento_tmp->inicio) >= tamanioTotal && segmento_tmp->id == 0;
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

//Creadores de segmentos segun tipo de dato
segmento_t* buscar_segmento(pcb_t pcb) {
	    //El segmento que se encontraba en memoria
		segmento_t* segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibrePcb);
		if ((segmentoMemoria->fin - segmentoMemoria->inicio) == sizeof(pcb_t)) {
					return segmentoMemoria;
				}
		segmento_t* nuevoSegmento = malloc(sizeof(segmento_t));
		uint32_t tamanioTotal = sizeof(pcb_t);
		nuevoSegmento->fin = segmentoMemoria->fin;
		segmentoMemoria->fin = tamanioTotal + segmentoMemoria->inicio; //YA CALCULE QUE ENTRA EN EL FIND
		segmentoMemoria->id = pcb.id;
		nuevoSegmento->inicio = segmentoMemoria->fin;
		nuevoSegmento->id = 0; //VACIO
		nuevoSegmento->tipoDato = VACIO;
		list_add(listaSegmentos,nuevoSegmento);
		segmentoMemoria->tipoDato = DATO_PCB;
		return segmentoMemoria;
}
segmento_t* buscar_segmentoTcb(tcb_t tcb,uint32_t patotaId) {
	    //El segmento que se encontraba en memoria
		segmento_t* segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibreTcb);
		if ((segmentoMemoria->fin - segmentoMemoria->inicio) == sizeof(tcb_t)) {
			return segmentoMemoria;
		}
		segmento_t* nuevoSegmento = malloc(sizeof(segmento_t));
		uint32_t tamanioTotal = sizeof(tcb_t); // + tamanio tareas
		nuevoSegmento->fin = segmentoMemoria->fin;
		segmentoMemoria->fin = tamanioTotal + segmentoMemoria->inicio; //YA CALCULE QUE ENTRA EN EL FIND
		segmentoMemoria->id = patotaId;
		nuevoSegmento->inicio = segmentoMemoria->fin;
		nuevoSegmento->id = 0; //VACIO
		nuevoSegmento->tipoDato = VACIO;
		list_add(listaSegmentos,nuevoSegmento);
		segmentoMemoria->tipoDato = DATO_TCB;
		return segmentoMemoria;
}
segmento_t* buscar_segmentoTareas(pcb_t pcb,char* tareas) {
	    //El segmento que se encontraba en memoria
		int tamanioTareas = strlen(tareas)*sizeof(char)+1;
		bool condicionSegmentoLibreTareas(void* segmento) {
			segmento_t* segmento_tmp = (segmento_t*)segmento;
			return (segmento_tmp->fin - segmento_tmp->inicio) >= tamanioTareas && segmento_tmp->id == 0;
		}
		segmento_t* segmentoMemoria = list_find(listaSegmentos,condicionSegmentoLibreTareas);
		if ((segmentoMemoria->fin - segmentoMemoria->inicio) == tamanioTareas) {
						return segmentoMemoria;
					}
		segmento_t* nuevoSegmento = malloc(sizeof(segmento_t));
		uint32_t tamanioTotal = tamanioTareas; // + tamanio tareas
		nuevoSegmento->fin = segmentoMemoria->fin;
		segmentoMemoria->fin = tamanioTotal + segmentoMemoria->inicio; //YA CALCULE QUE ENTRA EN EL FIND
		segmentoMemoria->id = pcb.id;
		nuevoSegmento->inicio = segmentoMemoria->fin;
		nuevoSegmento->id = 0; //VACIO
		nuevoSegmento->tipoDato = VACIO; //VACIO
		list_add(listaSegmentos,nuevoSegmento);
		segmentoMemoria->tipoDato = DATO_TAREAS;
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
	list_sort(listaSegmentos,ordenar_segun_inicio);
	segmentoAsignadoTareas = buscar_segmentoTareas(pcb_tmp,tareas);
	list_sort(listaSegmentos,ordenar_segun_inicio);
	offset = segmentoAsignado->inicio;
	memcpy(mem_ppal+offset,&pcb_tmp,sizeof(pcb_t));
	offset = segmentoAsignadoTareas->inicio;
	memcpy(mem_ppal+offset,tareas,strlen(tareas)*sizeof(char)+1);

	printf("Agregue a las listas de segmentos \n");
	while(creados < cant_trip) {
		segmento_t* segmentoTcb = malloc(sizeof(segmento_t));
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
			list_remove(listaSegmentos,i);
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
			printf("Pcb-> %i \n",pcb.id);
			break;
		}
		case DATO_TCB: {
			tcb_t tcb;
			memcpy(&tcb,mem_ppal+sg->inicio,sizeof(tcb_t));
			printf("Tcb-> %i X-> %i Y-> %i \n",tcb.id,tcb.x,tcb.y);
			break;
		}
		case DATO_TAREAS: {
			uint32_t tamanio = sg->fin - sg->inicio;
			char* str = malloc(tamanio*sizeof(char));
			printf("Inicio -> %i || Fin -> %i \n",sg->inicio,sg->fin);
			memcpy(str,mem_ppal+sg->inicio,tamanio*sizeof(char));
			printf("Tarea -> %s \n",str);

			break;
		}
		case VACIO: {
			printf("Segmento Libre \n");
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
			printf("Pcb-Proceso: %i \t\t Inicio: %i \t Fin:%i \t Tamanio:%i \t\n",sg->id,sg->inicio,sg->fin,tamanio);
			break;
		}
		case DATO_TCB: {
			printf("Tcb-Proceso: %i \t\t Inicio: %i \t Fin:%i \t Tamanio:%i \t\n",sg->id,sg->inicio,sg->fin,tamanio);
			break;
		}
		case DATO_TAREAS: {
			printf("Tareas-Proceso: %i \t Inicio: %i \t Fin:%i \t Tamanio:%i \t\n",sg->id,sg->inicio,sg->fin,tamanio);
			break;
		}
		case VACIO: {
			printf("Proceso: Libre \t\t Inicio: %i \t Fin:%i \t Tamanio:%i \t\n",sg->inicio,sg->fin,tamanio);
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
			memcpy(mem_ppal+sg->inicio,str,sizeof(tcb_t));
			break;
		case VACIO:
			break;
	}
	free(str);
	return 0;
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
		actual = list_get(listaSegmentos,i);
		i++;
		if(anterior->tipoDato == VACIO){
			offset += anterior->fin - anterior->inicio;
			desplazar_segmento(actual,offset);
			list_remove(listaSegmentos,i-1);
			cantidad--;
		}
		anterior = actual;
		actual = list_get(listaSegmentos,i);
		while (i < cantidad-1){
			if(actual->tipoDato == VACIO) {
				offset += actual->fin - actual->inicio;
				list_remove(listaSegmentos,i);
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
			list_remove(listaSegmentos,i);
			crear_segmento(tamanio-offset,tamanio);
		}
		return 0;
}
