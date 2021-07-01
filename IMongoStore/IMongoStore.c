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

	int libres = calcularEntradasLibres();
	log_info(logger,"libres = %d",libres);





	archivo.blocks = list_create();

	escribirEnMemoria("Tripulante termino tarea");

	leer_de_archivo("ads");


//	agregar_en_bloque("Tripulante termino tarea",8);
//
//	agregar_en_bloque("OOOOOOOOOOOOOO",5);
//
//	agregar_en_bloque("CCCCCC",5);
//
//
//	agregar_en_bloque("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",19);
//

	log_info(logger,"bloques:");

	obtener_bloque(2);

//
	obtener_bloque(0);
//
	obtener_bloque(1);


	munmap(fs_bloques, superblock.cantidad_bloques*superblock.tamanio_bloque);

	close(file_blocks);


	return EXIT_SUCCESS;
}


void iniciar_super_block(){
	//cargar desde config la struct de super_bloque
	log_info(logger,"Inicio superbloque: %s", conf_BYTES_BLOQUE);
	log_info(logger,"Tamanio de Bloque: %s", conf_BYTES_BLOQUE);
	log_info(logger,"Cantidad de Bloques: %s", conf_CANTIDAD_BLOQUES);

	superblock.tamanio_bloque = (uint32_t)atoi(conf_BYTES_BLOQUE);

	superblock.cantidad_bloques = atoi(conf_CANTIDAD_BLOQUES);
	char * bitmapstr = malloc(100);
	bitmap = crearBitArray(superblock.cantidad_bloques);

}

t_bitarray * crearBitArray(uint32_t cantBloques){

	int tamanioBitarray=cantBloques/8;
	if(cantBloques % 8 != 0){
	  tamanioBitarray++;
	 }

	char* bits=malloc(tamanioBitarray);

	t_bitarray * bitarray = bitarray_create_with_mode(bits,tamanioBitarray,MSB_FIRST);

	int cont=0;
	for(; cont < tamanioBitarray*8; cont++){
		bitarray_clean_bit(bitarray, cont);
	}

	return bitarray;
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

	printf("%s",plato_tmp.plato);

	return 1;
}


void iniciar_blocks(){
	file_blocks = open("block.ims", O_RDWR | O_CREAT , (mode_t)0600);

	int N=100;

	ftruncate(file_blocks,N*sizeof(t_bloque));

	fs_bloques = mmap ( NULL, superblock.tamanio_bloque * superblock.cantidad_bloques, PROT_READ | PROT_WRITE, MAP_SHARED , file_blocks, 0 );

}


int calcularCantidadDeEntradasAOcupar(char* palabra){
	int cantidadDeEntradas = string_length(palabra)/superblock.tamanio_bloque;

	if(string_length(palabra) % superblock.tamanio_bloque > 0 ){
		cantidadDeEntradas++;
	}

	return cantidadDeEntradas;
}


int devolverIndexParaAlmacenarValor(char * valor){
	int lugares = calcularCantidadDeEntradasAOcupar(valor);
	int cont = 0;
	int i;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(bitarray_test_bit(bitmap,i)){
			cont = 0;
		} else{
			cont++;
		}

		if(cont >= lugares){
			return i - lugares + 1;
		}
	}

	return 99999;
}


int calcularEntradasLibres(){

	int resultado = 0;
	int i;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		// Si la posicion del bitarray NO esta SETEADA es porque esta libre.
//		pthread_mutex_lock(&mutexStorage);
		if(!bitarray_test_bit(bitmap,i)){
			resultado++;
		}
//		pthread_mutex_unlock(&mutexStorage);
	}

	return resultado;
}

uint32_t escribirEnMemoria(char* valor){

	uint32_t resultado;

	int posicionesStorageAOcupar = calcularCantidadDeEntradasAOcupar(valor);
	int i;
	int inicioValor = 0;

	for(i=0; i < posicionesStorageAOcupar; i++){

		char* valorAux = string_substring(valor,inicioValor,superblock.tamanio_bloque);
		int posicion = devolverIndexParaAlmacenarValor(valorAux);

		agregar_en_bloque(valorAux,posicion);
		list_add(archivo.blocks,posicion);

		bitarray_set_bit(bitmap, posicion);
		inicioValor += superblock.tamanio_bloque;
	}
	return 1;

}

uint32_t leer_de_archivo(char* valor){

	int i;


	for(i=0; i < list_size(archivo.blocks); i++){

		int indice = list_get(archivo.blocks,i);
		obtener_bloque(indice);


	}
	printf("\n");
	return 1;

}
















