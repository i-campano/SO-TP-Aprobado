/*
 */

#ifndef IMONGOSTORE_H_
#define IMONGOSTORE_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include "utils.h"
#include "socket.h"

// CODIGO DE TAREAS
#define GENERAR_OXIGENO 90
#define GENERAR_COMIDA 91
#define GENERAR_BASURA 92
#define CONSUMIR_OXIGENO 110
#define CONSUMIR_COMIDA 111
#define DESCARTAR_BASURA 112





int server_fd;
char * archivo_oxigeno = "oxigeno.ims";
char * archivo_comida = "comida.ims";
char * archivo_basura = "basura.ims";

void manejadorDeHilos();
void inicializarMutex();

#endif /* IMONGOSTORE_H_ */
