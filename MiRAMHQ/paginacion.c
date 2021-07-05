#include "paginacion.h"
uint32_t tamanioPagina = 15;
uint32_t tamanioMemoria = 256;

uint32_t trip = 0;
int crear_memoria_paginacion(void){
	mem_ppalPag = malloc(tamanioMemoria);
	framesMemoria = list_create();
	tablasPatotaPaginacion = list_create();
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
int crear_patota_paginacion(uint32_t patota_id,char* tareas, uint32_t cantidad_tripulantes) {
	uint32_t tamanioTotal = sizeof(pcb_t) + sizeof(tcb_t) + string_length(tareas);
	uint32_t framesNecesarios = calcular_frames(tamanioTotal);
	printf("Frames necesarios -> %i\n",framesNecesarios);
	tabla_t* tablaPatota = buscar_frames(patota_id,framesNecesarios);
	pcb_t pcb;
	tcb_t tcb;
	pcb.id = patota_id;
	pcb.tareas = "8"; //ACA EMPIEZAN
	trip++;
	tcb.id = trip;

	tcb.x = 10;
	tcb.y = 11;
	uint32_t i = guardarPcb(tablaPatota,pcb);
	pcb_t pcb_temp;
	memcpy(&pcb_temp,mem_ppalPag,sizeof(pcb_t));
	printf("Guarde %i bytes Valor-> Id -> %i\n",i,pcb_temp.id);

	i = guardarTareas(tablaPatota,tareas);
	char* tarea = malloc(strlen(tareas));
	memcpy(tarea,mem_ppalPag+8,strlen(tareas));
	printf("Guarde %i bytes -> %s \n",i,tareas);
	memcpy(&pcb_temp,mem_ppalPag,sizeof(pcb_t));
	printf("---->Id -> %i\n",pcb_temp.id);


	i = guardarTcb(tablaPatota,tcb,0);
	tcb_t tcb_temp;
	memcpy(&tcb_temp,mem_ppalPag+(8+strlen(tareas)),sizeof(tcb_t));
	printf("Guarde %i bytes ID: %i X: %i Y: %i\n",i,tcb_temp.id,tcb_temp.x,tcb_temp.y);
	list_add(tablasPatotaPaginacion,tablaPatota);
	return 0;
}
tabla_t* buscar_frames(uint32_t id,uint32_t framesNecesarios) {
	tabla_t* tablaPatota = malloc(sizeof(tabla_t));
	uint32_t framesAsignados = 0;
	tablaPatota->idPatota = id;
	tablaPatota->listaAsignados = list_create();
	//BUSCAR CUANTOS FRAMES LIBRES HAY SI NO TENGO NECESARIOS INFORMAR
	while(framesAsignados < framesNecesarios) {
		frame_t* frame = list_find(framesMemoria,condicionFrameLibre);
		frame->estado = false;
		frame->nPagina = framesAsignados;
		pagina_t* pagina = malloc(sizeof(pagina_t));
		pagina->Nframe = frame->numeroFrame;
		list_add(tablaPatota->listaAsignados,pagina);
		framesAsignados++;
	}
	return tablaPatota;
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
void mostrarFrames(void* frame){
	frame_t* fr = (frame_t*)frame;
	if(fr->estado){
		printf("Frame: %i Estado: Libre \n",fr->numeroFrame);
	}
	else printf("Frame: %i Estado: Ocupado Npagina: %i  \n",fr->numeroFrame,fr->nPagina);
}
uint32_t guardarPcb(tabla_t* tabla,pcb_t pcb) {
	uint32_t guardado = 0;
	uint32_t i = 0;

	while (guardado < sizeof(pcb_t) && (i+1)*tamanioPagina <=  sizeof(pcb_t)) {
		pagina_t* pagina = list_get(tabla->listaAsignados,i);
		uint32_t offset = pagina->Nframe * tamanioPagina;
		void* temp = (void*)&pcb;
		memcpy(mem_ppalPag+offset,temp+guardado,tamanioPagina);
		guardado += tamanioPagina;
		i++;
	}
	if(guardado == sizeof(pcb_t)) {
		return guardado;
	}
	pagina_t* pagina = list_get(tabla->listaAsignados,i);
	uint32_t offset = pagina->Nframe * tamanioPagina;
	void* temp = (void*)&pcb;
	memcpy(mem_ppalPag+offset,temp + guardado,sizeof(pcb_t)-guardado);
	guardado += sizeof(pcb_t)-guardado;
	return guardado;

}
uint32_t guardarTareas(tabla_t* tabla,char* tareas) {
	uint32_t nPagina = sizeof(pcb_t) / tamanioPagina;
	pagina_t* pagina = list_get(tabla->listaAsignados,nPagina);
	uint32_t offset = pagina->Nframe*tamanioPagina;
	uint32_t tamanioTareas = string_length(tareas);
	uint32_t librePag = tamanioPagina;
	uint32_t guardado = 0;
	tabla->tamanioTareas = tamanioTareas;

	if(sizeof(pcb_t) % tamanioPagina){
		if(sizeof(pcb_t) > tamanioPagina){
		offset += sizeof(pcb_t) % tamanioPagina;
		librePag = tamanioPagina - (sizeof(pcb_t) % tamanioPagina);
		}
		else {
		offset += sizeof(pcb_t);
		librePag = tamanioPagina - sizeof(pcb_t);
		}
	}
	if(librePag >= tamanioTareas) {
		memcpy(mem_ppalPag+offset,tareas,tamanioTareas);
		guardado += tamanioTareas;
		return guardado;
	}
	memcpy(mem_ppalPag+offset,tareas,librePag);
	guardado += librePag;
	librePag = tamanioPagina;
	nPagina++;
	while(guardado < tamanioTareas){
		pagina = list_get(tabla->listaAsignados,nPagina);
		offset = pagina->Nframe*tamanioPagina;
		if(librePag > tamanioTareas-guardado){
			memcpy(mem_ppalPag+offset,tareas+guardado,tamanioTareas-guardado);
			guardado += tamanioTareas-guardado;
			return guardado;
		}
		memcpy(mem_ppalPag+offset,tareas+guardado,librePag);
		guardado += librePag;
		nPagina++;
		librePag = tamanioPagina;
	}
	return guardado;
}

//nTripulante es el numero de tripulante en la patota
uint32_t guardarTcb(tabla_t* tabla,tcb_t tcb,uint32_t nTripulante) {
	uint32_t ocupado = tabla->tamanioTareas + sizeof(pcb_t);
	uint32_t nPagina = ocupado / tamanioPagina;
		pagina_t* pagina = list_get(tabla->listaAsignados,nPagina);
		uint32_t offset = pagina->Nframe*tamanioPagina;
		uint32_t librePag;
		uint32_t guardado = 0;

		void* aux = (void*)&tcb;
		if(ocupado % tamanioPagina && ocupado > tamanioPagina){
			offset += ocupado % tamanioPagina;
			librePag = tamanioPagina - (ocupado % tamanioPagina);
		}
		else {
			offset += ocupado;
			librePag = tamanioPagina - ocupado;
		}
		if(librePag >= sizeof(tcb_t)) {
			memcpy(mem_ppalPag+offset,aux,sizeof(tcb_t));
			guardado += sizeof(tcb_t);
			return guardado;
		}
		memcpy(mem_ppalPag+offset,aux,librePag);
		guardado += librePag;
		librePag = tamanioPagina;
		nPagina++;
		while(guardado < sizeof(tcb_t)){
			pagina = list_get(tabla->listaAsignados,nPagina);
			offset = pagina->Nframe*tamanioPagina;
			if(librePag > sizeof(tcb_t)-guardado){
				memcpy(mem_ppalPag+offset,aux+guardado,sizeof(tcb_t)-guardado);
				guardado += sizeof(tcb_t)-guardado;
				return guardado;
			}
			memcpy(mem_ppalPag+offset,aux+guardado,librePag);
			guardado += librePag;
			nPagina++;
			librePag = tamanioPagina;
		}
		return guardado;
	}

pcb_t getPcbPaginacion(uint32_t id_patota) {
	bool tablaPatotaPaginacion(void* tabla) {
		tabla_t* tablaPatota = (tabla_t*)tabla;
		return tablaPatota->idPatota == id_patota;
	}
	tabla_t* tabla = list_find(tablasPatotaPaginacion,tablaPatotaPaginacion);
	uint32_t leido = 0;
	uint32_t i = 0;
	void* temp = malloc(sizeof(tcb_t));
		while (leido < sizeof(pcb_t) && (i+1)*tamanioPagina <=  sizeof(pcb_t)) {
			pagina_t* pagina = list_get(tabla->listaAsignados,i);
			uint32_t offset = pagina->Nframe * tamanioPagina;
			memcpy(temp+leido,mem_ppalPag+offset,tamanioPagina);
			leido += tamanioPagina;
			i++;
		}
		if(leido == sizeof(pcb_t)) {
			pcb_t pcb = *(pcb_t*)temp;
			printf("Leido -> %i \n",leido);
			free(temp);
			return pcb;
		}
		pagina_t* pagina = list_get(tabla->listaAsignados,i);
		uint32_t offset = pagina->Nframe * tamanioPagina;
		printf("Pagina -> %i offset-> %i \n",i,offset);
		memcpy(temp + leido,mem_ppalPag+offset,sizeof(pcb_t)-leido);
		leido += sizeof(pcb_t)-leido;
		pcb_t pcb = *(pcb_t*)temp;
		printf("Leido sobro espacio -> %i \n",leido);
		free(temp);
		return pcb;

}

tcb_t getTcbPaginacion(uint32_t id_trip, uint32_t id_patota){
	bool tablaPatotaPaginacion(void* tabla) {
		tabla_t* tablaPatota = (tabla_t*)tabla;
		return tablaPatota->idPatota == id_patota;
	}
	tabla_t* tabla = list_find(tablasPatotaPaginacion,tablaPatotaPaginacion);
	uint32_t ocupado = sizeof(pcb_t)+tabla->tamanioTareas;
			uint32_t nPagina = ocupado / tamanioPagina;
			pagina_t* pagina;
			uint32_t offset = 0;
			uint32_t librePag = tamanioPagina;
			uint32_t leido = 0;
			tcb_t tcb;
			void* aux = malloc(sizeof(tcb_t));
			if(ocupado % tamanioPagina) {
				if(tamanioPagina > ocupado){
					offset += ocupado;
					librePag -= ocupado;
				}
				else {
					offset += ocupado % tamanioPagina;
					librePag -= ocupado % tamanioPagina;
				}
			}
			while(leido < sizeof(tcb_t) && (sizeof(tcb_t)-leido) >= tamanioPagina){
				pagina = list_get(tabla->listaAsignados,nPagina);
				offset = pagina->Nframe*tamanioPagina + offset;
				printf("Leido: %i Offset: %i Pagina: %i \n",leido,offset,nPagina);
				memcpy(aux+leido,mem_ppalPag+offset,librePag);
				leido += librePag;
				librePag = tamanioPagina;
				offset = 0;
				nPagina++;
			}
			if(leido < sizeof(tcb_t)){
				pagina = list_get(tabla->listaAsignados,nPagina);
				offset = pagina->Nframe*tamanioPagina + offset;
				printf("IF - >Leido: %i Offset: %i Pagina: %i \n",leido,offset,nPagina);
				memcpy(aux+leido,mem_ppalPag+offset,sizeof(tcb_t)-leido);
				leido += sizeof(tcb_t)-leido;
			}
			tcb = *(tcb_t*)aux;
			free(aux);
			return tcb;
	}

char* getTareas(uint32_t id_patota){
	bool tablaPatotaPaginacion(void* tabla) {
			tabla_t* tablaPatota = (tabla_t*)tabla;
			return tablaPatota->idPatota == id_patota;
		}
		tabla_t* tabla = list_find(tablasPatotaPaginacion,tablaPatotaPaginacion);
		uint32_t ocupado = sizeof(pcb_t);
		uint32_t nPagina = ocupado / tamanioPagina;
		pagina_t* pagina;
		uint32_t offset = 0;
		uint32_t librePag = tamanioPagina;
		uint32_t leido = 0;
		char* tarea = malloc(tabla->tamanioTareas + 1);
		if(ocupado % tamanioPagina) {
			if(tamanioPagina > ocupado){
				offset += ocupado;
				librePag -= ocupado;
			}
			else {
				offset += ocupado % tamanioPagina;
				librePag -= ocupado % tamanioPagina;
			}
		}
		while(leido < tabla->tamanioTareas){
			pagina = list_get(tabla->listaAsignados,nPagina);
			offset = pagina->Nframe*tamanioPagina + offset;
			printf("Leido: %i Offset: %i Pagina: %i \n",leido,offset,nPagina);
			memcpy(tarea+leido,mem_ppalPag+offset,librePag);
			leido += librePag;
			librePag = tamanioPagina;
			offset = 0;
			nPagina++;
		}
		tarea[tabla->tamanioTareas] = '\0';
		return tarea;
}
char* reconocer_tareas2(char* tarea,uint32_t tareaPedida){
	char anterior;
	char actual;
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
