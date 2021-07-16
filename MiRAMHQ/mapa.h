#include <nivel-gui/nivel-gui.h>
#include <nivel-gui/tad_nivel.h>
#include <stdlib.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
t_list* tripulantes;
t_list* nuevos;
pthread_t mapaHilo;
sem_t nuevo;
pthread_mutex_t accesoNuevos;
pthread_mutex_t accesoCreados;
sem_t actualizar_pos;
typedef struct {
	bool modificado;
	int x;
	int y;
	char idNivel;
	int id;
	int dx;
	int dy;
}trip_t;

int err;
NIVEL* nivel;
int cols, rows;
int iniciar_mapa(void);
void* mapa(void* arg);
void* actualizar_tripulante(void* arg);
trip_t* buscar_tripulanteIdMap(int id);
bool buscarTripulanteActualizar(void* dato);
void* crear_tripulanteMap(void* arg);
