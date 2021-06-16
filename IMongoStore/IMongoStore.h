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
char * archivo_oxigeno = "oxigeno.ims";
char * archivo_comida = "comida.ims";
char * archivo_basura = "basura.ims";

void manejadorDeHilos();

#endif /* IMONGOSTORE_H_ */
