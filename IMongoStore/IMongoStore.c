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
//	inicializarMutex();
	//Inicializacion del loger
	logger = log_create("IMongoStore.log", "IMongoStore", 1, LOG_LEVEL_DEBUG);

	//Inicializacion del archivo de Configuracion
//	iniciar_configuracion();

//	server_fd = iniciarServidor(conf_PUERTO_IMONGO);

	//iniciarEstructurasAdministrativas();

//	log_info(logger, "Servidor listo para recibir al cliente");

	//Inicia escucha de llamados
	//manejadorDeHilos();

//	iniciaEstructuraDeArchivos();


	/*escribirBitacora("resuelve sabotaje",1);
	escribirBitacora("resuelve sabotaje",2);
	escribirBitacora("resuelve sabotaje",3);
	escribirBitacora("se mueve saraza22",5);*/


//	log_info(logger,"finished");

/*	generarOxigeno(10);
	consumirOxigeno(5);

	generarBasura(8);
	descartarBasura();

	ejecutarTarea("GENERAR_OXIGENO",10);
	ejecutarTarea("GENERAR_BASURA",10);*/
	iniciar_configuracion();
	iniciar_super_block();
//
	iniciar_blocks();
	//uso la info del superblock para crear el archivo de blocks
//	iniciar_blocks();
//
//	str_metadata oxigeno = crear_archivo();
//	str_metadata basura = crear_archivo();
//	str_metadata comida = crear_archivo();
//
//
//	//En el server cuando atiendo a los tripulantes
//	//En cada hilo
//	str_metadata bitacora = crear_archivo();
//
//	platos_agregar("otromassss",11,8);
//
//	platos_agregar("seg",32,5);

//	escribir();

//	leer();

	obtener_plato(2);

	obtener_plato(8);

	munmap(bloques, 100*sizeof(t_plato_comanda));

	close(blocks);
	return EXIT_SUCCESS;
}

int leer(void) {
  int fd = open("test_file", O_RDWR | O_CREAT, (mode_t)0600);
  char *map = mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  char text[20];
  memcpy(text, map, 10);
	log_info(logger,"%s",text);
  close(fd);
  return 0;
}


int escribir(void) {
  int fd = open("test_file", O_RDWR | O_CREAT, (mode_t)0600);
  const char *text = "hello";
  size_t textsize = strlen(text) + 1;
  lseek(fd, textsize-1, SEEK_SET);
  write(fd, "", 1);
  char *map = mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  memcpy(map, text, strlen(text));
  msync(map, textsize, MS_SYNC);
  munmap(map, textsize);
  close(fd);
  return 0;
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

int platos_agregar(char * cadena_caracteres,uint32_t cantidad,int indice) {
	t_plato_comanda plato_tmp;
	bzero(&plato_tmp, 32);

	// copiar a estructura temporal
	strcpy(plato_tmp.plato, cadena_caracteres);
	plato_tmp.cantidad = cantidad;

	// Copiar la estructura en el espacio libre
	memcpy(bloques + (indice*32), &plato_tmp, 32);
	msync(bloques, 10*32, MS_SYNC);

	return 1;
}


int obtener_plato(int indice) {
	t_plato_comanda plato_tmp;
	bzero(&plato_tmp, 32);

	// Copiar la estructura en el espacio libre
	memcpy(&plato_tmp,bloques + (indice*32), 32);

	log_info(logger,"%s",plato_tmp.plato);
	log_info(logger,"%d",plato_tmp.cantidad);

	return 1;
}


void iniciar_blocks(){
	blocks = open("otroblock.ims", O_RDWR | O_CREAT , (mode_t)0600);




	int N=100;
	ftruncate(blocks,N*sizeof(t_plato_comanda));

	bloques = mmap ( NULL, N*sizeof(t_plato_comanda), PROT_READ | PROT_WRITE, MAP_SHARED , blocks, 0 );

}


















