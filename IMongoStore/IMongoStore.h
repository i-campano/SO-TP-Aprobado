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

#define IP "127.0.0.1"
#define PUERTO "5002"

int server_fd;

void manejadorDeHilos();

#endif /* IMONGOSTORE_H_ */
