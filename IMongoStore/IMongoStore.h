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
void escribirArchivo(FILE archivo, char letra, uint32_t cantidad);
void generarOxigeno(uint32_t cantidad);
void consumirOxigeno(uint32_t cantidad);
void generarComida(uint32_t cantidad);
void consumirComida(uint32_t cantidad);
void generarBasura(uint32_t cantidad);
void descartarBasura();

#endif /* IMONGOSTORE_H_ */
