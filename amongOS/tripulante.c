/*#include "tripulante.h"

int estado_planificacion; //VARIABLE GLOBAL
nodo_tripulante_t* lista_new = NULL;
nodo_tripulante_t* lista_ready = NULL;
nodo_tripulante_t* lista_exec = NULL;
nodo_tripulante_t* lista_bloq = NULL;
nodo_tripulante_t* lista_fin = NULL;


void* planificador_new(void) {
				//De new a ready no depende de la planificacion en RR FIFO
				while (estado_planificacion == ACTIVE){
					if (lista_new->dato->tarea != NULL) {
						//YA sabe su tarea, esta ready
						lista_new->dato->estado = READY;
						//AGREGO EL 1ERO A LA LISTA DE READY
						//ELIMINO EL DATO DE LA LISTA DE NEW
					}
					if (estado_planificacion == PAUSE) {
						//QUIZAS SE INFORMA A ALGUIEN
						while (estado_planificacion == PAUSE){
							//QUEDATE EN BUCLE SIN HACER NADA
						}
						//SE REANUDA PLANIFICACION
					}
				}
	return 0;
}
//Son N hilos de ready?
void* planificador_ready(void* arg) {
				config_planif* configuracion;
				configuracion = (config_planif*) arg;
				while (estado_planificacion == ACTIVE){
					if (configuracion->trip_en_exec < configuracion->grado_multitareas){
						//AGREGO 1 READY A EXEC
						//REVISAR CUANDO ASIGANAR ESTO DE ABAJO
						lista_ready->dato->estado = EXEC;
						//ELIMINO DE READY A DATO
						configuracion->trip_en_exec++;
					}
					if (estado_planificacion == PAUSE) {
						//QUIZAS SE INFORMA A ALGUIEN
						while (estado_planificacion == PAUSE){
							//QUEDATE EN BUCLE SIN HACER NADA
						}
						//SE REANUDA PLANIFICACION
					}
				}
	return 0;
}
void* planificador_exec(void* arg) {
				config_planif* configuracion;
				configuracion = (config_planif*) arg;
				while (estado_planificacion == ACTIVE){
					//Agarro 1 tripulante de EXEC
					//Hago que trabaje
					while(lista_exec->dato->trabajando == TRUE){
					}
					//Termino sus tareas
					if (lista_exec->dato->fin_tareas == TRUE){
						lista_exec->dato->estado = FIN;
						//AGREGO A FIN
						//ELIMINO DE EXEC
					}
					else {
						lista_exec->dato->estado = BLOQ;
						//AREGO A BLOQUEDOS
						//TE ELIMINO DE EXEC
					}
					if (estado_planificacion == PAUSE) {
						//QUIZAS SE INFORMA A ALGUIEN
						//PAUSO LAS TAREAS DE LOS TRIP Y GUARDO EL ESTADO EN QUE ESTABAN
						while (estado_planificacion == PAUSE){
							//QUEDATE EN BUCLE SIN HACER NADA
						}
						//SE REANUDA PLANIFICACION
					}
				}
	return 0;
}
*/
