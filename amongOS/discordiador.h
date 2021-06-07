#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include "utils.h"

#define RAM 0
#define FILE_SYSTEM 1


#define DISCORDIADOR 2
#define MIRAM 9

#define PATOTA_CREADA 22

#define CREAR_PATOTA 4






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
