/*
 * sabotaje.h
 *
 *  Created on: 13 jul. 2021
 *      Author: utnso
 */

#ifndef SABOTAJE_H_
#define SABOTAJE_H_


#include "utils.h"

void mostrar_bloques(t_list * lista_bloques);

void igualar_bitmap_contra_bloques(t_list * bloques_ocupados);

void sabotaje_bitmap_superbloque();



void bloques_file_bitacora(_archivo_bitacora * archivo,t_list * lista_bloques);

void bloques_ocupados_file(_archivo * archivo,t_list * lista_bloques);

void fsck();


void adulterar_bitmap(int signal);


#endif /* SABOTAJE_H_ */
