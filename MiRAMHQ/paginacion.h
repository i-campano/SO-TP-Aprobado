#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "ADMIN_MIRAM.h"

// Excelente!
typedef struct {
	uint32_t numeroFrame;
	bool estado;
	uint32_t recentUsed;
	uint32_t nPagina;
}frame_t;

//TAMBIEN PARA PAGINACION!
typedef struct {
	uint32_t idPatota;
	t_list* listaAsignados;
	uint32_t tamanioTareas;
}tabla_t;
//VER SI SON NECESARIOS MAS CAMPOS
typedef struct {
	uint32_t Nframe;

}pagina_t;
//VARIABLES GLOBALES ------------------------
t_list* framesMemoria;
t_list* tablasPatotaPaginacion;
void* mem_ppalPag;
//LISTAS -------___>
bool condicionFrameLibre(void* valor);
void mostrarFrames(void* frame);
//--------------------------------
tabla_t* buscar_frames(uint32_t id,uint32_t framesNecesarios);
tabla_t* crear_tabla_patota_paginacion(uint32_t id,uint32_t framesNecesarios);
uint32_t calcular_frames(uint32_t tamanioTotal);
int crear_memoria_paginacion(void);
int crear_patota_paginacion(uint32_t patota_id,char* tareas, uint32_t cantidad_tripulantes);

uint32_t guardarTcb(tabla_t* tabla,tcb_t tcb,uint32_t nTripulante);
uint32_t guardarTareas(tabla_t* tabla,char* tareas);
uint32_t guardarPcb(tabla_t* tabla,pcb_t pcb);

pcb_t getPcbPaginacion(uint32_t id_patota);
tcb_t getTcbPaginacion(uint32_t id_trip, uint32_t id_patota);
char* getTareas(uint32_t id_patota);
char* reconocer_tareas2(char* tarea,uint32_t tareaPedida);
