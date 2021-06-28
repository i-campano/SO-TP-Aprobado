
#ifndef ADMIN_MIRAM_H_
#define ADMIN_MIRAM_H_
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
#include "utils.h"

#define DATO_PCB 0
#define DATO_TCB 1
#define DATO_TAREAS 2
#define VACIO 3
#define FF 0
#define BF 1

typedef struct {
	uint32_t inicio;
	uint32_t tipoDato; //Que dato hay?
	uint32_t id; //A que proceso corresponde?
	uint32_t fin;
}segmento_t;
typedef struct {
	uint32_t dirBase;
	uint32_t tipoDato; //Que dato hay?
	uint32_t id; //A que proceso corresponde?
	uint32_t tamanio;
}tablaSegmento_t;
typedef struct{
	uint32_t nFrame;
	bool estado;
}frame_t;
typedef struct{
	uint32_t nPagina;
	uint32_t id_patota;
}pagina_t;
typedef struct {
	uint32_t id;
	char* tareas;
}pcb_t;

typedef struct {
	uint32_t id;
	char estado;
	uint32_t x;
	uint32_t y;
	char* prox_tarea;
//	pcb_t* pcb;
	uint32_t pcb;
}tcb_t;

t_list* listaSegmentos;
t_list* tablaSegmentos;
t_list* listaTablaSegmentos;

pthread_mutex_t accesoMemoria;
pthread_mutex_t accesoListaSegmentos;
//PPAL
int admin_memoria(void);
//FUNCIONES
void crear_memoria_ppal();

//Criterios para listas de segmentos
bool condicionSegmentoLibrePcb(void* segmento);
bool ordenar_segun_inicio(void* primero,void* segundo);
bool condicionSegmentoLibreTcb(void* segmento);
void* condicionSegmentoLibreTcbBF(void* segmento,void* otroSegmento);
void* condicionSegmentoLibrePcbBF(void* segmento,void* otroSegmento);

//Creadores de segmentos segun tipo de dato
segmento_t* buscar_segmento(pcb_t pcb);
segmento_t* buscar_segmentoTcb(tcb_t tcb,uint32_t patotaId);
segmento_t* buscar_segmentoTareas(pcb_t pcb,char* tareas);
t_list* buscarTablaPatota(uint32_t id);
//ELIMINAR Y RECIBIR TAREAS (Creacion y borrar segmentos)
void crear_patota(uint32_t cant_trip,char* tareas);
int unificar_sg_libres(void);
void liberar_segmento(segmento_t* sg);
int eliminar_patota(int id);
void crear_segmento(uint32_t inicio,uint32_t fin);

//ELIMINANDO
int unificar_sg_libres(void);
void liberar_segmento(segmento_t* sg);
int eliminar_patota(int id);
int compactar_memoria(void);

//PARA ITEREAR EN LAS LISTAS Y MOSTRAR COSAS
void mostrarMemoriaCompleta(void* segmento);
void mostrarEstadoMemoria(void* segmento);

//Funciones compactacion
int desplazar_segmento(segmento_t* sg,uint32_t offset);
int compactar_memoria(void);
int memoria_libre(void);
//PROBANDO MERGEAR CON MIRAM POSTA
void crear_patota2(pcb_t pcb,char* posiciones,char* tareas);
void crear_tripulante(uint32_t idTrip,uint32_t id_patota,uint32_t x, uint32_t y,uint32_t idpatota);
//Get
pcb_t getPcb (int idPedido);
tcb_t getTcb (int idPedido);
char* getTarea(int idPedido,uint32_t nTarea);
void setPcb(pcb_t pcb);
void setTcb (int idPedido,tcb_t tcb);

char* reconocer_tareas(char* tarea,uint32_t tareaPedida);
#endif /* ADMIN_MIRAM_H_ */
