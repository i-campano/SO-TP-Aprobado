
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

#define DATO_PCB 0
#define DATO_TCB 1
#define DATO_TAREAS 2
#define VACIO 3

typedef struct {
	uint32_t inicio;
	uint32_t tipoDato; //Que dato hay?
	uint32_t id; //A que proceso corresponde?
	uint32_t fin;
}segmento_t;
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
	pcb_t* pcb;
}tcb_t;
//FUNCIONES
void crear_memoria_ppal();

//Criterios para listas de segmentos
bool condicionSegmentoLibrePcb(void* segmento);
bool ordenar_segun_inicio(void* primero,void* segundo);
bool condicionSegmentoLibreTcb(void* segmento);

//Creadores de segmentos segun tipo de dato
segmento_t* buscar_segmento(pcb_t pcb);
segmento_t* buscar_segmentoTcb(tcb_t tcb,uint32_t patotaId);
segmento_t* buscar_segmentoTareas(pcb_t pcb,char* tareas);

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

#endif /* ADMIN_MIRAM_H_ */
