#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_
#define RAM 0
#define FILE_SYSTEM 1

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <pthread.h>

#include "utils.h"

int realizar_operacion(char* mensaje,int conexion_mi_ram,int conexion_file_system,int conexion_tripulante);
t_log* iniciar_logger(void);
t_config* leer_config(void);
void leer_consola(t_log* logger,int conexion_ram,int conexion_fs,int conexion_trip);
t_paquete* armar_paquete();
int terminar_programa(t_log* logger,t_config* config,int conexion[2]);
#endif /* DISCORDIADOR_H_ */
