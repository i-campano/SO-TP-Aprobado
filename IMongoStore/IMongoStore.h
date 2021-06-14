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


int server_fd;
char * archivo_oxigeno = "Oxigeno.ims";
char * archivo_comida = "Comida.ims";
char * archivo_basura = "Basura.ims";

void manejadorDeHilos();

#endif /* IMONGOSTORE_H_ */
