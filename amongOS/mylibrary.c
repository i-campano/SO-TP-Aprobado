
#include "mylibrary.h"


char diccionario[CANT_OPERATION][MAX_LENGTH_HEAD] = 	{MSG_INIT_PLANIF,
													MSG_PAUSE_PLANIF,
													MSG_RESUME_PLANIF,
													MSG_NEW_PATOTA,
													MSG_LIST_CREW,
													MSG_EXP_CREW,
													MSG_BITACORA
	};

char* obtener_primera_palabra(char* mensaje) {
	char* primera_palabra = malloc(sizeof(char)*MAX_LENGTH_HEAD+1);
	int i = 0;
	if (mensaje == NULL){
		free(primera_palabra);
		return NULL;
	}
	memset(primera_palabra,0,sizeof(char)*MAX_LENGTH_HEAD+1);
	while(mensaje[i] != ' '){
		primera_palabra[i] = mensaje[i];
		i++;
	}
	return primera_palabra;
}
int reconocer_op_code(char* mensaje) {
	char* primera_palabra;
	short int i = 0;

	if (mensaje == NULL){
		return -1;
	}
	primera_palabra = obtener_primera_palabra(mensaje);
	while(strcmp(primera_palabra,diccionario[i])&& i < CANT_OPERATION) {
		i++;
	}
	free(primera_palabra);
	return i;
}
