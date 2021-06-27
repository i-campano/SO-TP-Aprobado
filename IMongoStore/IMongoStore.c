/*
 */

#include "IMongoStore.h"

void inicializarMutex() {
	pthread_mutex_init(&mut_ARCHIVO_OXIGENO, NULL);
	pthread_mutex_init(&mut_ARCHIVO_COMIDA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_BASURA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_OXIGENO_METADATA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_COMIDA_METADATA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_BASURA_METADATA, NULL);
	pthread_mutex_init(&mut_ARCHIVO_BLOCKS, NULL);
	pthread_mutex_init(&mut_ARCHIVO_SUPERBLOQUE, NULL);
}

int main(void)
{

	logger = log_create("IMongoStore.log", "IMongoStore", 1, LOG_LEVEL_DEBUG);

	iniciar_configuracion();
	iniciar_super_block();
	iniciar_blocks();

//	str_metadata oxigeno = crear_archivo();
//	str_metadata basura = crear_archivo();
//	str_metadata comida = crear_archivo();


//	En el server cuando atiendo a los tripulantes
//	En cada hilo
//	str_metadata bitacora = crear_archivo();


	//Recibe cadena a insertar e indice de bloque
	agregar_en_bloque("Tripulante termino tarea",8);

	agregar_en_bloque("OOOOOOOOO",5);

	agregar_en_bloque("CCCCCCCCC",5);

	obtener_bloque(8);

	obtener_bloque(5);

	munmap(fs_bloques, 100*sizeof(t_bloque));

	close(blocks);
	return EXIT_SUCCESS;
}


void iniciar_super_block(){
	//cargar desde config la struct de super_bloque
	log_info(logger,conf_BYTES_BLOQUE);
	log_info(logger,conf_CANTIDAD_BLOQUES);

	superblock.tamanio_bloque = (uint32_t)atoi(conf_BYTES_BLOQUE);

	superblock.cantidad_bloques = atoi(conf_CANTIDAD_BLOQUES);
	void * bitmapstr = malloc(10);
	superblock.bitmap = bitarray_create_with_mode(bitmapstr,superblock.cantidad_bloques,LSB_FIRST);

	log_info(logger,"%s",superblock.bitmap->bitarray);

}

int agregar_en_bloque(char * cadena_caracteres,int indice) {

	t_bloque bloque;
	bzero(&bloque, sizeof(t_bloque));

	// copiar a estructura temporal
	strcpy(bloque.plato, cadena_caracteres);

	// Copiar la estructura en el espacio libre
	memcpy(fs_bloques + (indice*sizeof(t_bloque)), &bloque, sizeof(t_bloque));
	msync(fs_bloques, 100*sizeof(t_bloque), MS_SYNC);

	return 1;
}


int obtener_bloque(int indice) {
	t_bloque plato_tmp;
	bzero(&plato_tmp, sizeof(t_bloque));

	// Copiar la estructura en el espacio libre
	memcpy(&plato_tmp,fs_bloques + (indice*sizeof(t_bloque)), sizeof(t_bloque));

	log_info(logger,"%s",plato_tmp.plato);

	return 1;
}


void iniciar_blocks(){
	blocks = open("otroblock.ims", O_RDWR | O_CREAT , (mode_t)0600);

	int N=100;

	ftruncate(blocks,N*sizeof(t_bloque));

	fs_bloques = mmap ( NULL, N*sizeof(t_bloque), PROT_READ | PROT_WRITE, MAP_SHARED , blocks, 0 );

}


















