#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include "mylibrary.h"
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<readline/readline.h>
#include <pthread.h>
#include <semaphore.h>


char* ip_mongo;
char* ip_miram;
int puerto_mongo;
int puerto_miram;
char* valor;




#define PATOTA_CREADA 22
#define ACTUALIZACION_IMONGOSTORE 15
#define CREAR_PATOTA 4

#define HANDSHAKE_TRIPULANTE 5

#define PEDIR_TAREA 7

#define CREAR_TRIPULANTE 10

#define TRIPULANTE_CREADO 11

#define ENVIAR_TAREA 8

#define EJECUTAR_TAREA 9


t_log* logger;
int socketListener,socketMaximo;	 // PARA SERVER
fd_set socketClientes, socketClientesAuxiliares; // PARA SERVER

int socketServerMiRam;

int socketServerIMongoStore;

pthread_mutex_t comuni;

t_list * hilosParaConexiones;

t_queue* planificacion_cola_new;
t_queue* planificacion_cola_ready;
t_queue* planificacion_cola_exec;
t_queue* planificacion_cola_bloq;
t_queue* planificcion_cola_fin;

//HILOS
pthread_t hiloConsola;
pthread_t hiloCoordinador;
pthread_t hiloPlanificador;

//HILOS PARA COLAS
pthread_t hiloColaReady;
pthread_t hiloColaNew;

//SEMAFORO CONSUMIDOR PRODUCTOR
sem_t cola_ready;
sem_t cola_new;

//MUTEX PARA COLAS
pthread_mutex_t planificacion_mutex_new;
pthread_mutex_t planificacion_mutex_ready;
pthread_mutex_t planificacion_mutex_exec;
pthread_mutex_t planificacion_mutex_bloq;
pthread_mutex_t planificcion_mutex_fin;

//Semaforos
pthread_mutex_t mutexHilos;

sem_t iniciar_planificacion;
sem_t detenerReaunudarEjecucion;


//sem de tipo cola, N = grado multiprogramacion
sem_t exec;


sem_t activar_actualizaciones_mongo;


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
	sem_t ready;
	sem_t new;
	sem_t exec;
}t_nodo_tripulante;



typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


void iniciar_configuracion();

void iniciarEstructurasAdministrativasPlanificador();


int terminar_programa(t_log* logger,t_config* config,int conexion[2]);

void iniciar_logger();

t_config* leer_config();


#endif /* UTILS_H_ */
