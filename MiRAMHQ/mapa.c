
#include "mapa.h"
#define ASSERT_CREATE(nivel, id, err)                                                   \
    if(err) {                                                                           \
        nivel_destruir(nivel);                                                          \
        nivel_gui_terminar();                                                           \
        fprintf(stderr, "Error al crear '%c': %s\n", id, nivel_gui_string_error(err));  \
        return EXIT_FAILURE;                                                            \
    }

int iniciar_mapa(void){
	pthread_create(&mapaHilo , NULL, mapa , NULL);
	return 0;
}
void* mapa(void* arg) {
	pthread_t crearTrip;
	pthread_t actualizarTrip;
	nuevos = list_create();
	tripulantes = list_create();
	sem_init(&nuevo, 0, 0);
	sem_init(&actualizar_pos, 0, 0);
	pthread_mutex_init(&accesoNuevos,NULL);
	pthread_mutex_init(&accesoCreados,NULL);
	pthread_create(&crearTrip , NULL, crear_tripulanteMap , NULL);
	pthread_create(&actualizarTrip , NULL, actualizar_tripulante , NULL);
	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&cols, &rows);

	nivel = nivel_crear("Nave_AFIRM");


	while ( 1 ) {
		nivel_gui_dibujar(nivel);

		if(err) {
			printf("WARN: %s\n", nivel_gui_string_error(err));
		}
	}

}
void* crear_tripulanteMap(void* arg) {
	while(1){
		sem_wait(&nuevo);
		pthread_mutex_lock(&accesoNuevos);
		trip_t* tripulante = list_remove(nuevos,0);
		pthread_mutex_unlock(&accesoNuevos);
		err = personaje_crear(nivel,tripulante->id,tripulante->x, tripulante->y);
				ASSERT_CREATE(nivel,tripulante->id, err);
		pthread_mutex_lock(&accesoCreados);
		list_add(tripulantes,tripulante);
		pthread_mutex_unlock(&accesoCreados);
	}
	return 0;
}
bool buscarTripulanteActualizar(void* dato) {
	trip_t* tripulante = (trip_t*) dato;
	return tripulante->modificado;
}

trip_t* buscar_tripulanteIdMap(int id){
	bool buscarId(void* dato) {
		trip_t* tripulante = (trip_t*) dato;
		return tripulante->id == id;
	}
	return list_find(tripulantes,buscarId);
}
void* actualizar_tripulante(void* arg){
	while(1) {
	sem_wait(&actualizar_pos);
	pthread_mutex_lock(&accesoCreados);
	trip_t* tripulante = list_find(tripulantes,buscarTripulanteActualizar);
	pthread_mutex_unlock(&accesoCreados);
	tripulante->modificado = false;
	item_desplazar(nivel,tripulante->id, tripulante->dx,tripulante->dy);
	tripulante->x += tripulante->dx;
	tripulante->y += tripulante->dy;

	}
	return 0;
}


