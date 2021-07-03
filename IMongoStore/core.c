#include "core.h"

void iniciar_super_block(){
	//cargar desde config la struct de super_bloque
	log_info(logger,"Inicio superbloque: %s", conf_BYTES_BLOQUE);
	log_info(logger,"Tamanio de Bloque: %s", conf_BYTES_BLOQUE);
	log_info(logger,"Cantidad de Bloques: %s", conf_CANTIDAD_BLOQUES);

	superblock.tamanio_bloque = (uint32_t)atoi(conf_BYTES_BLOQUE);

	superblock.cantidad_bloques = atoi(conf_CANTIDAD_BLOQUES);

	superblock.bitmapstr = malloc(superblock.cantidad_bloques);

	superblock.bitmap = crear_bit_array(superblock.cantidad_bloques);

}

t_bitarray * crear_bit_array(uint32_t cantBloques){

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

int write_blocks(char * cadena_caracteres,int indice) {

	t_bloque bloque;
	bzero(&bloque, sizeof(t_bloque));

	strcpy(bloque.data, cadena_caracteres);

	memcpy(_blocks.fs_bloques + (indice*sizeof(t_bloque)), &bloque, sizeof(t_bloque));
	msync(_blocks.fs_bloques, superblock.cantidad_bloques*sizeof(t_bloque), MS_SYNC);

	return 1;
}

int agregar_en_archivo(char * cadena_caracteres,int indice, _archivo archivo) {

	t_registros_metadata registros_archivo;
	bzero(&registros_archivo, sizeof(t_registros_metadata));

	strcpy(registros_archivo.campo, cadena_caracteres);

	memcpy(archivo.contenido + (indice*sizeof(t_registros_metadata)), &registros_archivo, sizeof(t_registros_metadata));
	msync(archivo.contenido, 100*sizeof(t_registros_metadata), MS_SYNC);

	return 1;
}

int leer_metadata_archivo(char * cadena_caracteres,int indice, _archivo archivo) {
	//	implementacion con diccionario  ¿¿

	t_registros_metadata registros_archivo;
	bzero(&registros_archivo, sizeof(t_registros_metadata));

	memcpy(&registros_archivo.campo,archivo.contenido + (indice*sizeof(t_registros_metadata)), sizeof(t_registros_metadata));
	msync(archivo.contenido, 100*sizeof(t_registros_metadata), MS_SYNC);

	log_info(logger,"Metadata : %s",registros_archivo.campo);

	return 1;
}

int obtener_contenido_bloque(int indice,char ** bloqueReturned) {
	t_bloque bloque;
	bzero(&bloque, sizeof(t_bloque));

	memcpy(&bloque,_blocks.fs_bloques + (indice*sizeof(t_bloque)), sizeof(t_bloque));

//	printf("%s",bloque.data);
	string_append(bloqueReturned,(bloque.data));

	return 1;
}

int obtener_bloque(int indice) {
	t_bloque bloque;
	bzero(&bloque, sizeof(t_bloque));

	memcpy(&bloque,_blocks.fs_bloques + (indice*sizeof(t_bloque)), sizeof(t_bloque));

	printf("%s",bloque.data);

	return 1;
}

void iniciar_blocks(){
	_blocks.file_blocks = open("block.ims", O_RDWR | O_CREAT , (mode_t)0600);

	ftruncate(_blocks.file_blocks,superblock.cantidad_bloques*sizeof(t_bloque));

	_blocks.fs_bloques = mmap ( NULL, superblock.tamanio_bloque * superblock.cantidad_bloques, PROT_READ | PROT_WRITE, MAP_SHARED , _blocks.file_blocks, 0 );

}

void iniciar_archivo(char * name_file,_archivo *archivo,char * key_file){
	archivo->clave = string_new();
	string_append(&(archivo->clave),key_file);

	(*archivo).blocks = list_create();

	pthread_mutex_init(&(*archivo).mutex_file, NULL);


	(*archivo).file = open(name_file, O_RDWR | O_CREAT , (mode_t)0600);

	int N=100;

	ftruncate((*archivo).file,N*sizeof(t_bloque));

	(*archivo).contenido = mmap ( NULL, superblock.tamanio_bloque * superblock.cantidad_bloques, PROT_READ | PROT_WRITE, MAP_SHARED ,(*archivo).file, 0 );

}

int calcular_bloques_libres(){

	int resultado = 0;
	int i;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(!bitarray_test_bit(superblock.bitmap,i)){
			resultado++;
		}
	}

	return resultado;
}

int calcular_cantidad_bloques_requeridos(char* cadenaAGuardar){
	int cantidadBloques = string_length(cadenaAGuardar)/superblock.tamanio_bloque;

	if(string_length(cadenaAGuardar) % superblock.tamanio_bloque > 0 ){
		cantidadBloques++;
	}

	return cantidadBloques;
}

int obtener_indice_para_guardar_en_bloque(char * valor){
	int lugares = calcular_cantidad_bloques_requeridos(valor);
	int cont = 0;
	int i;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(bitarray_test_bit(superblock.bitmap,i)){
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

void actualizar_metadata(_archivo archivo,int indice_bloque,char * valorAux){

	list_add(archivo.blocks,indice_bloque);

	int cantidad_bloques = list_size(archivo.blocks);

	char * cantidad = string_from_format("%d",cantidad_bloques);

	agregar_en_archivo("cantidad bloques",0,archivo);

	agregar_en_archivo(cantidad,1,archivo);

	agregar_en_archivo("bloques",2,archivo);

	//agregar indice a la lista de bloques


}


void actualizar_metadata_borrado(_archivo * archivo){

	int cantidad_bloques = list_size(archivo->blocks);

	char * cantidad = string_from_format("%d",cantidad_bloques);

	agregar_en_archivo("cantidad bloques",0,*archivo);

	agregar_en_archivo(cantidad,1,*archivo);

	agregar_en_archivo("bloques",2,*archivo);

	//agregar indice a la lista de bloques


}

uint32_t write_archivo(char* valor,_archivo archivo){

	uint32_t resultado;

	int posicionesStorageAOcupar = calcular_cantidad_bloques_requeridos(valor);
	int i;
	int inicioValor = 0;

	//chequear si hay lugar en el ultimo bloque antes de agregar uno nuevo

	for(i=0; i < posicionesStorageAOcupar; i++){

		char* valorAux = string_substring(valor,inicioValor,superblock.tamanio_bloque);

		int indice_bloque = obtener_indice_para_guardar_en_bloque(valorAux);

		write_blocks(valorAux,indice_bloque);

		actualizar_metadata(archivo,indice_bloque,valorAux);

		bitarray_set_bit(superblock.bitmap, indice_bloque);

		inicioValor += superblock.tamanio_bloque;
	}
	return 1;

}

void remover_bloque(int indice,_archivo * archivo){
	list_remove(archivo->blocks,indice);
	bitarray_clean_bit(superblock.bitmap, indice);
	actualizar_metadata_borrado(archivo);

}

void consumir_arch(_archivo * archivo,int cantidadAConsumir){

	int ultimo = (int)list_get(archivo->blocks,list_size(archivo->blocks)-1);

	char * contenidoBloque = string_new();

	obtener_contenido_bloque(ultimo,&contenidoBloque);

	int longitudBloque = string_length(contenidoBloque);

	while(cantidadAConsumir>=longitudBloque){

		remover_bloque(ultimo,archivo);

		cantidadAConsumir-=longitudBloque;

		ultimo = (int)list_get(archivo->blocks,list_size(archivo->blocks)-1);
		free(contenidoBloque);
		contenidoBloque = string_new();
		obtener_contenido_bloque(ultimo,&contenidoBloque);
		longitudBloque = string_length(contenidoBloque);

	}
	if(cantidadAConsumir>0){
		free(contenidoBloque);
		contenidoBloque = string_new();
		obtener_contenido_bloque(ultimo,&contenidoBloque);
		contenidoBloque = string_substring_until(contenidoBloque,string_length(contenidoBloque)-cantidadAConsumir);

		write_blocks(contenidoBloque,ultimo);

	}


}

uint32_t leer_contenido_archivo(char * c,_archivo * archivo){
	int i;

	log_info(logger,"Contenido del archivo: %s",archivo->clave);
	for(i=0; i < list_size(archivo->blocks); i++){

		int indice = list_get(archivo->blocks,i);
		obtener_bloque(indice);

	}
	printf("\n");
	return 1;

}
