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

	memcpy(_blocks.fs_bloques + (indice*sizeof(t_bloque)), &(bloque.data), sizeof(t_bloque));
	msync(_blocks.fs_bloques, (indice*sizeof(t_bloque)), MS_SYNC);

	return 1;
}

int write_blocks_with_offset(char * cadena_caracteres,int indice,int offset) {

	t_bloque bloque;
	bzero(&bloque, sizeof(t_bloque));

	strcpy(bloque.data, cadena_caracteres);

	memcpy(_blocks.fs_bloques + (indice*sizeof(t_bloque))+offset, &(bloque.data), sizeof(t_bloque));
	msync(_blocks.fs_bloques, (indice*sizeof(t_bloque)), MS_SYNC);

	return 1;
}

int clean_block(char * cadena_caracteres,int indice) {

	t_bloque bloque;
	bzero(&bloque, sizeof(t_bloque));

	memcpy(_blocks.fs_bloques + (indice*sizeof(t_bloque)), &(bloque.data), sizeof(t_bloque));
	msync(_blocks.fs_bloques, (indice*sizeof(t_bloque)), MS_SYNC);

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
	archivo->metadata = malloc(10000);
	archivo->metadata = config_create(name_file);

	config_set_value(archivo->metadata,"CARACTER_LLENADO","O");

	config_set_value(archivo->metadata,"MD5","XXXX");
	config_set_value(archivo->metadata,"SIZE","0");
	config_set_value(archivo->metadata,"BLOCKS","[]");
	config_set_value(archivo->metadata,"BLOCK_COUNT","0");


	pthread_mutex_init(&(*archivo).mutex_file, NULL);

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

void actualizar_metadata(_archivo * archivo,int indice_bloque,char * valorAux){

	char * md5 = string_new();
	md5 = config_get_string_value (archivo->metadata, "MD5");

	string_length(valorAux);
	int bytes = string_length(valorAux);

	int size = config_get_int_value(archivo->metadata,"SIZE");
	size+=bytes;
	config_set_value(archivo->metadata,"SIZE",string_itoa(size));


	char ** array;
	array = config_get_array_value(archivo->metadata,"BLOCKS");
	char ** nuevo  = agregar_en_array(array,string_itoa(indice_bloque));

	config_set_value(archivo->metadata,"BLOCK_COUNT",string_itoa(longitud_array(nuevo)));

	char * cadena = array_to_string(nuevo);
	int i = 0;
	while(nuevo[i]!=NULL){
		free(nuevo[i]);
		i++;
	}
	free(nuevo[i]);
	free(nuevo);
	config_set_value(archivo->metadata,"BLOCKS",cadena);
	config_save(archivo->metadata);
}

void actualizar_metadata_sin_crear_bloque(_archivo * archivo,char * valorAux){

	char * md5 = string_new();
	md5 = config_get_string_value (archivo->metadata, "MD5");

	string_length(valorAux);
	int bytes = string_length(valorAux);

	int size = config_get_int_value(archivo->metadata,"SIZE");
	size+=bytes;
	config_set_value(archivo->metadata,"SIZE",string_itoa(size));
	config_save(archivo->metadata);

}

void actualizar_metadata_borrado(_archivo * archivo,int cantidadABorrar){

	int size = config_get_int_value(archivo->metadata,"SIZE");
	size-=cantidadABorrar;
	config_set_value(archivo->metadata,"SIZE",string_itoa(size));
	config_save(archivo->metadata);

}

void actualizar_metadata_elimina_bloque(_archivo * archivo,int cantidadABorrar){

	char ** blocks = config_get_array_value(archivo->metadata,"BLOCKS");
	int block_count = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
	int size = config_get_int_value(archivo->metadata,"SIZE");
	blocks[block_count-1]=NULL;

	char * bloques_str = array_to_string(blocks);

	config_set_value(archivo->metadata,"BLOCKS",bloques_str);
	config_set_value(archivo->metadata,"BLOCK_COUNT",string_itoa(block_count-1));
	config_set_value(archivo->metadata,"SIZE",string_itoa(size-cantidadABorrar));
	config_save(archivo->metadata);

}

uint32_t write_archivo(char* valor,_archivo * archivo){

	uint32_t resultado;
	int bytesArchivo = config_get_int_value(archivo->metadata,"SIZE");

	if(bytesArchivo%superblock.tamanio_bloque==0){



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
	}else{
		while(bytesArchivo>superblock.tamanio_bloque) bytesArchivo-=superblock.tamanio_bloque;
		char ** blocks = config_get_array_value(archivo->metadata,"BLOCKS");
		int count_block = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
		char * last_block = blocks[count_block-1];
		if(string_length(valor)<=superblock.tamanio_bloque-bytesArchivo){

			write_blocks_with_offset(valor,atoi(last_block),bytesArchivo);
			actualizar_metadata_sin_crear_bloque(archivo,valor);
		}else{
			char * primera_parte  = string_substring_until(valor,superblock.tamanio_bloque-bytesArchivo);
			char * desde = string_substring_from(valor,superblock.tamanio_bloque-bytesArchivo);
			write_blocks_with_offset(primera_parte,atoi(last_block),bytesArchivo);
			actualizar_metadata_sin_crear_bloque(archivo,primera_parte);

			int posicionesStorageAOcupar = calcular_cantidad_bloques_requeridos(desde);
			int i;
			int inicioValor = 0;

			//chequear si hay lugar en el ultimo bloque antes de agregar uno nuevo

			for(i=0; i < posicionesStorageAOcupar; i++){

				char* valorAux = string_substring(desde,inicioValor,superblock.tamanio_bloque);

				int indice_bloque = obtener_indice_para_guardar_en_bloque(valorAux);

				write_blocks(valorAux,indice_bloque);

				actualizar_metadata(archivo,indice_bloque,valorAux);

				bitarray_set_bit(superblock.bitmap, indice_bloque);

				inicioValor += superblock.tamanio_bloque;
			}
		}
	}
	return 1;

}

void remover_bloque(int indice,_archivo * archivo, int cantidadAConsumir){
	bitarray_clean_bit(superblock.bitmap,indice);
	clean_block("asd,b",indice);

	actualizar_metadata_elimina_bloque(archivo,cantidadAConsumir);

}

void consumir_arch(_archivo * archivo,int cantidadAConsumir){

	char ** bloques = config_get_array_value(archivo->metadata,"BLOCKS");
	int cantidad_bloques = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
	int size = config_get_int_value(archivo->metadata,"SIZE");
	cantidad_bloques--;
	char * bloque = bloques[cantidad_bloques];

	char * contenidoBloque = string_new();

	int indice = atoi(bloque);
	obtener_contenido_bloque(indice,&contenidoBloque);

	int longitudBloque = string_length(contenidoBloque);
//borrar el primero que si supera
	while((cantidadAConsumir>0)){

		if(cantidadAConsumir>longitudBloque){

			cantidadAConsumir-=longitudBloque;
			remover_bloque(indice,archivo,longitudBloque);
			contenidoBloque = string_new();
			cantidad_bloques--;
			bloque = bloques[cantidad_bloques];
			indice = atoi(bloque);
			obtener_contenido_bloque(indice,&contenidoBloque);
			longitudBloque = string_length(contenidoBloque);
		}
		else if(cantidadAConsumir==longitudBloque){
			cantidadAConsumir-=longitudBloque;
			remover_bloque(indice,archivo,longitudBloque);
		}
		else if (cantidadAConsumir<longitudBloque){
			actualizar_metadata_borrado(archivo,cantidadAConsumir);

			contenidoBloque = string_substring_until(contenidoBloque,string_length(contenidoBloque)-cantidadAConsumir);

			write_blocks(contenidoBloque,indice);
			cantidadAConsumir=0;
		}

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
