#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_
#define RAM 0
#define FILE_SYSTEM 1

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include <pthread.h>
#include "tripulante.h"
#include "socket.h"

#include "utils.h"


t_log* logger;
int socketServerMiRam;	// PARA USUARIO
int socketListener,socketMaximo;	 // PARA SERVER
fd_set socketClientes, socketClientesAuxiliares; // PARA SERVER

t_list * hilosParaConexiones;


t_queue* planificacion_cola_new = NULL;
t_queue* planificacion_cola_ready = NULL;
t_queue* planificacion_cola_exec = NULL;
t_queue* planificacion_cola_bloq = NULL;
t_queue* planificcion_cola_fin = NULL;


//HILOS
pthread_t hiloConsola;
pthread_t hiloCoordinador;
pthread_t hiloPlanificador;

//Semaforos
pthread_mutex_t mutexHilos;

typedef struct _infoHilos{
	int socket;
	pthread_t hiloAtendedor;
} infoHilos;

typedef struct {
	int id;
	char estado;
	short int trabajando;
	short int fin_tareas;
	char* tarea; //Calculo que es necesario 50 50 SEGURIDAD
	pthread_t hilo_asociado;
}t_nodo_tripulante;

void iniciarEstructurasAdministrativasPlanificador();


//Funciones para los hilos
void planificar_tripulantes();
void leer_consola();
void atender_ram();

//Funciones que crean hilos
void atenderLaRam();
void iniciarHiloConsola();




//Funciones administrativas - loggeo - configuracion
void iniciarEstructurasAdministrativasPlanificador();

void iniciar_logger(void);
t_config* leer_config(void);
void leer_consola2(t_log* logger,int conexion_ram,int conexion_fs,int conexion_trip);
t_paquete* armar_paquete();
int terminar_programa(t_log* logger,t_config* config,int conexion[2]);




//Funciones PARA REVISAR o PARA IMPLEMENTAR (NO SE USAN)
int realizar_operacion(char* mensaje,int conexion_mi_ram,int conexion_file_system,int conexion_tripulante);
void iniciar_patota(char* mensaje);
void *labor_tripulante (void* tripulante);
#endif /* DISCORDIADOR_H_ */
