#include "core.h"



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


void exit_failure() {
	perror("Error: ");
	exit(EXIT_FAILURE);
}
void iniciar_blocks(){

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	_blocks.file_blocks = open("block.ims", O_RDWR | O_CREAT | O_TRUNC, mode);

	if (_blocks.file_blocks == (-1)) {
		perror("open");
		exit_failure();
	}
	ftruncate(_blocks.file_blocks,superblock.cantidad_bloques*superblock.tamanio_bloque);

	_blocks.fs_bloques = malloc(superblock.cantidad_bloques*superblock.tamanio_bloque);
	_blocks.original_blocks = malloc(superblock.cantidad_bloques * superblock.tamanio_bloque);

	bzero(_blocks.fs_bloques,superblock.cantidad_bloques * superblock.tamanio_bloque);
	bzero(_blocks.original_blocks,superblock.cantidad_bloques * superblock.tamanio_bloque);

	_blocks.original_blocks = mmap ( NULL, superblock.tamanio_bloque * superblock.cantidad_bloques, PROT_READ | PROT_WRITE, MAP_SHARED , _blocks.file_blocks, 0 );

	pthread_mutex_init(&_blocks.mutex_blocks,NULL);

}

int write_blocks(char * cadena_caracteres,int indice) {

	int padding = superblock.tamanio_bloque - strlen(cadena_caracteres);
	char * pad = string_repeat('#',padding);
	char * cadena  = string_duplicate(cadena_caracteres);
	string_append(&cadena,pad);
	

	memcpy(_blocks.fs_bloques + (indice*superblock.tamanio_bloque), cadena, superblock.tamanio_bloque);

	return 1;
}

int write_blocks_with_offset(char * cadena_caracteres,int indice,int offset) {


	int padding = superblock.tamanio_bloque - offset-strlen(cadena_caracteres);
	char * pad = string_repeat('#',padding);
	char * cadena  = string_duplicate(cadena_caracteres);
	string_append(&cadena,pad);


//	TODO : meter la validacion de bitarray aca  ¿
	memcpy(_blocks.fs_bloques + (indice*superblock.tamanio_bloque)+offset, cadena, string_length(cadena));
	return 1;
}

int clean_block(int indice) {

	char * clean = string_repeat('*',superblock.tamanio_bloque);

	memcpy(_blocks.fs_bloques + (indice*superblock.tamanio_bloque), clean, superblock.tamanio_bloque);
	log_info(logger,"clean_block(): Blocks: %s",_blocks.fs_bloques);
	return 1;
}

//TODO: refactor obtener ->return string
char * obtener_contenido_bloque(int indice) {
	char * bloque = (char*)malloc(superblock.tamanio_bloque);
	bzero(bloque,superblock.tamanio_bloque);
//	TODO: PORQUE TAMANIO -1 ¿¿
	memcpy(bloque,_blocks.fs_bloques + (indice*superblock.tamanio_bloque), 8);

	return bloque;
}




//--------------------------SYNC----------------------------------------

void sincronizar_blocks(){
	while(1){
		sleep(conf_TIEMPO_SINCRONIZACION);
		log_trace(logger,"SINCRONIZANDO DISCO");
		pthread_mutex_lock(&_blocks.mutex_blocks);
		pthread_mutex_lock(&superblock.mutex_superbloque);
		log_trace(logger,"SINCRO - MUTEX_BLOCKS - BLOCKED");
		log_debug(logger,"SYNC: COPIA blocks.ims: %s",_blocks.fs_bloques);
		memcpy(_blocks.original_blocks, (_blocks.fs_bloques), (superblock.cantidad_bloques*superblock.tamanio_bloque));
		msync(_blocks.original_blocks, (superblock.cantidad_bloques*superblock.tamanio_bloque), MS_SYNC);
		log_debug(logger,"SYNC: blocks.ims: %s",_blocks.original_blocks);
		log_trace(logger,"SINCRO  - MUTEX_BLOCKS - UNBLOCKED");

		log_info(logger,"SINCRO  bitmap - BLOCKED");



		memcpy(superblock.bitmapstr + 2*sizeof(uint32_t), (superblock.bitmap->bitarray), (superblock.cantidad_bloques/8));
		msync(superblock.bitmapstr, 2*sizeof(uint32_t)+ (superblock.cantidad_bloques/8), MS_SYNC);

		pthread_mutex_unlock(&superblock.mutex_superbloque);
		pthread_mutex_unlock(&_blocks.mutex_blocks);

		log_info(logger,"SINCRO  bitmap - unBLOCKED");
	}
}

void hilo_sincronizar_blocks(){
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1, PTHREAD_CREATE_DETACHED);
	pthread_t hilo = (pthread_t)malloc(sizeof(pthread_t));
	pthread_create(&hilo , &attr1,(void*) sincronizar_blocks,NULL);
}


//---------------------------METADATA------------------------------------------------

int calcular_cantidad_bloques_requeridos(char* cadenaAGuardar){
	int cantidadBloques = string_length(cadenaAGuardar)/superblock.tamanio_bloque;

	if(string_length(cadenaAGuardar) % superblock.tamanio_bloque > 0 ){
		cantidadBloques++;
	}

	return cantidadBloques;
}


//--------------------------SUPER BLOQUE--------------------------------------------
void iniciar_super_block(){
	//cargar desde config la struct de super_bloque
	log_info(logger,"iniciar_super_blocks(): Iniciando superblock");
	log_info(logger,"Inicio superbloque: %s", conf_BYTES_BLOQUE);
	log_info(logger,"Tamanio de Bloque: %s", conf_BYTES_BLOQUE);
	log_info(logger,"Cantidad de Bloques: %s", conf_CANTIDAD_BLOQUES);

	superblock.tamanio_bloque = (uint32_t)atoi(conf_BYTES_BLOQUE);

	superblock.cantidad_bloques = atoi(conf_CANTIDAD_BLOQUES);

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	int tamanioBitarray=superblock.cantidad_bloques/8;
	if(superblock.cantidad_bloques % 8 != 0){
	  tamanioBitarray++;
	}
	int tamanioFs = tamanioBitarray+(sizeof(uint32_t))*2;

//	if( access( "superblock.ims", F_OK ) == 0 ) {
//		log_debug(logger,"iniciar_super_blocks(): Iniciando con superblock existente");
//		superblock.file_superblock = open("superblock.ims", O_RDWR | O_CREAT , mode);
//	}else{
	log_debug(logger,"iniciar_super_blocks(): Creando archivo de superblock");
	superblock.file_superblock = open("superblock.ims", O_RDWR | O_CREAT , mode);
	ftruncate(superblock.file_superblock,tamanioFs);
//	}



	superblock.bitmap = crear_bit_array(superblock.cantidad_bloques);


	superblock.bitmapstr = malloc(superblock.cantidad_bloques * superblock.tamanio_bloque);
	bzero(superblock.bitmapstr,tamanioFs);

	superblock.bitmapstr = mmap ( NULL, tamanioFs, PROT_READ | PROT_WRITE, MAP_SHARED , superblock.file_superblock, 0 );

	uint32_t cantidad_bloques_fs = *(uint32_t*)(superblock.bitmapstr);


	//TODO : Buscar otra validacion para confirmar que existe un fs.
	if(cantidad_bloques_fs!=superblock.cantidad_bloques || 1){
		ftruncate(superblock.file_superblock,tamanioFs);
		log_debug(logger,"Creando metadata de FS...");


		memcpy(superblock.bitmapstr, &(superblock.cantidad_bloques), sizeof(uint32_t));
		memcpy(superblock.bitmapstr+sizeof(uint32_t), &(superblock.tamanio_bloque), sizeof(uint32_t));


		memcpy(superblock.bitmapstr+(sizeof(uint32_t))*2, (superblock.bitmap->bitarray),tamanioBitarray);
		msync(superblock.bitmapstr+ (sizeof(uint32_t))*2, tamanioFs, MS_SYNC);

		log_debug(logger,"Metadata creada...");
	}else{
		log_debug(logger,"Usando FS existente...");
	}


	pthread_mutex_init(&(superblock.mutex_superbloque),NULL);

}

int calcular_bloques_libres_ONLY(){
	pthread_mutex_lock(&superblock.mutex_superbloque);
	int resultado = 0;
	int i;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(!bitarray_test_bit(superblock.bitmap,i)){
			resultado++;
		}
	}
	pthread_mutex_unlock(&superblock.mutex_superbloque);
	return resultado;
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
	log_trace(logger,"calcular_bloques_libres: %d",resultado);
	return resultado;
}



int obtener_indice_para_guardar_en_bloque(char * valor){
	int lugares = calcular_cantidad_bloques_requeridos(valor);
	int cont = 0;
	int i;
	int resultado = 0;
	int cantidadDePosiciones = superblock.cantidad_bloques;

	for(i=0;i<cantidadDePosiciones;i++){

		if(bitarray_test_bit(superblock.bitmap,i)){
			cont = 0;
		} else{
			cont++;
		}
		if(cont >= lugares){
			return i - lugares + 1;
			break;
		}
	}
	return 99999;
}


void iniciar_archivo(char * name_file,_archivo **archivo,char * key_file,char * caracter_llenado){
	*archivo = (_archivo*)malloc(sizeof(_archivo));
	(*archivo)->metadata = malloc(sizeof(t_config));
	(*archivo)->clave = string_new();
	string_append(&((*archivo)->clave),key_file);

	(*archivo)->blocks = list_create();

	t_config *config;
	if((config = config_create(name_file))!=NULL) {
		FILE * metadata = open(name_file, O_RDWR | O_CREAT | O_TRUNC , (mode_t)0600);
		ftruncate(metadata,1000);
	}

	(*archivo)->metadata = config_create(name_file);

	config_set_value((*archivo)->metadata,"CARACTER_LLENADO",caracter_llenado);

	config_set_value((*archivo)->metadata,"MD5","XXXX");
	config_set_value((*archivo)->metadata,"SIZE","0");
	config_set_value((*archivo)->metadata,"BLOCKS","[]");
	config_set_value((*archivo)->metadata,"BLOCK_COUNT","0");

	config_save((*archivo)->metadata);

	pthread_mutex_init(&((*archivo)->mutex_file), NULL);


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
	char * md5;
	md5 = config_get_string_value (archivo->metadata, "MD5");
	log_debug(logger,"MD5: %s",md5);

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

void bitarray_set_bit_monitor(int indice_bloque) {
	//TODO: set bit a funcion
	bitarray_set_bit(superblock.bitmap, indice_bloque);
}

void llenar_nuevo_bloque(char* cadenaAGuardar, _archivo* archivo) {
	//Si el ultiimo bloque esta completo -> creo los bloques nuevos que necesito para almacenar la cadena
	int posicionesStorageAOcupar = calcular_cantidad_bloques_requeridos(
			cadenaAGuardar);
	int offsetBytesAlmacenados = 0;
	int bloqueslibres = calcular_bloques_libres();
	for (int i = 0; i < posicionesStorageAOcupar; i++) {
		char* valorAux = string_substring(cadenaAGuardar,
				offsetBytesAlmacenados, superblock.tamanio_bloque);
		int indice_bloque = obtener_indice_para_guardar_en_bloque(valorAux);
		log_trace(logger, "llenar_nuevo_bloque(): File_Recurso: %s -> bloque asignado: %d", archivo->clave,
				indice_bloque);
		write_blocks(valorAux, indice_bloque);
		actualizar_metadata(archivo, indice_bloque, valorAux);
		bitarray_set_bit_monitor(indice_bloque);
		offsetBytesAlmacenados += superblock.tamanio_bloque;
	}
}

uint32_t write_archivo(char* cadenaAGuardar,_archivo * archivo){
	pthread_mutex_lock(&(archivo->mutex_file));

	uint32_t resultado;
	int bytesArchivo = config_get_int_value(archivo->metadata,"SIZE");

	log_trace(logger,"bytes archivo %s : %d",archivo->clave,bytesArchivo);

	pthread_mutex_lock(&_blocks.mutex_blocks);
	pthread_mutex_lock(&superblock.mutex_superbloque);
	//Chequeo si hay lugar en el ultimo bloque
	if(bytesArchivo%superblock.tamanio_bloque==0){

		//Si el ultiimo bloque esta completo -> creo los bloques nuevos que necesito para almacenar la cadena
		llenar_nuevo_bloque(cadenaAGuardar, archivo);

	}else{
		//bytesArchivo : restas sucesivas para quedarme con el espacio OCUPADO del ultimo bloque.
		while(bytesArchivo>superblock.tamanio_bloque) bytesArchivo-=superblock.tamanio_bloque;

		int espacioLibreUltimoBloque = superblock.tamanio_bloque-bytesArchivo;

		char ** blocks = config_get_array_value(archivo->metadata,"BLOCKS");

		int count_block = config_get_int_value(archivo->metadata,"BLOCK_COUNT");

		char * last_block = blocks[count_block-1];

		if(string_length(cadenaAGuardar)<=espacioLibreUltimoBloque){

			log_trace(logger,"indice de BLOQUE :%d con espacio para archivo: %s, ",atoi(last_block),archivo->clave);

			write_blocks_with_offset(cadenaAGuardar,atoi(last_block),bytesArchivo);

			//TODO: podria armar un actualizar metadata mas generico con varios if
			actualizar_metadata_sin_crear_bloque(archivo,cadenaAGuardar);

		}else{

			char * rellenoDeUltimoBloque  = string_substring_until(cadenaAGuardar,espacioLibreUltimoBloque);

			write_blocks_with_offset(rellenoDeUltimoBloque,atoi(last_block),bytesArchivo);

			actualizar_metadata_sin_crear_bloque(archivo,rellenoDeUltimoBloque);

			char * contenidoProximoBloque = string_substring_from(cadenaAGuardar,espacioLibreUltimoBloque);

			llenar_nuevo_bloque(contenidoProximoBloque, archivo);

		}
	}
	log_trace(logger,"write_archivo()->Recurso: %s - Copia blocks.ims: %s ",archivo->clave,_blocks.fs_bloques);
	pthread_mutex_unlock(&(superblock.mutex_superbloque));
	pthread_mutex_unlock(&(_blocks.mutex_blocks));
	pthread_mutex_unlock(&(archivo->mutex_file));
	return 1;
}

void remover_bloque(int indice,_archivo * archivo, int cantidadAConsumir){

	bitarray_clean_bit(superblock.bitmap,indice);

	clean_block(indice);

	actualizar_metadata_elimina_bloque(archivo,cantidadAConsumir);

}

void consumir_arch(_archivo * archivo,int cantidadAConsumir){
	pthread_mutex_lock(&(archivo->mutex_file));

	char ** bloques = config_get_array_value(archivo->metadata,"BLOCKS");
	int cantidad_bloques = config_get_int_value(archivo->metadata,"BLOCK_COUNT");
	int size = config_get_int_value(archivo->metadata,"SIZE");
	cantidad_bloques--;
	char * bloque = bloques[cantidad_bloques];

	int sizeUltimoBloque = size;
	while(sizeUltimoBloque>superblock.tamanio_bloque) sizeUltimoBloque-=superblock.tamanio_bloque;

	char * contenidoBloque = string_new();

	int indice = atoi(bloque);

	pthread_mutex_lock(&_blocks.mutex_blocks);
	log_trace(logger,"consumir_arch() - MUTEX_BLOCKS - BLOCKED");
	pthread_mutex_lock(&superblock.mutex_superbloque);
	log_trace(logger,"consumir_arch() - MUTEX_SUPERBLOQUE - BLOCKED");
	contenidoBloque = obtener_contenido_bloque(indice);

	int longitudBloque = 0 ;

	while((cantidadAConsumir>0)){

		if(cantidadAConsumir>sizeUltimoBloque){

			cantidadAConsumir-=sizeUltimoBloque;
			remover_bloque(indice,archivo,sizeUltimoBloque);
			contenidoBloque = string_new();
			cantidad_bloques--;
			bloque = bloques[cantidad_bloques];
			indice = atoi(bloque);
			contenidoBloque = obtener_contenido_bloque(indice);
			longitudBloque = string_length(contenidoBloque);
			sizeUltimoBloque =longitudBloque;
		}
		else if(cantidadAConsumir==sizeUltimoBloque){
			cantidadAConsumir-=sizeUltimoBloque;
			remover_bloque(indice,archivo,sizeUltimoBloque);
		}
		else if (cantidadAConsumir<sizeUltimoBloque){
			actualizar_metadata_borrado(archivo,cantidadAConsumir);

			contenidoBloque = string_substring_until(contenidoBloque,sizeUltimoBloque-cantidadAConsumir);

			write_blocks(contenidoBloque,indice);
			cantidadAConsumir=0;
		}

	}
	log_trace(logger,"consumir_arch()->Recurso: %s - Copia blocks.ims: %s ",archivo->clave,_blocks.fs_bloques);
	pthread_mutex_unlock(&(superblock.mutex_superbloque));
	pthread_mutex_unlock(&(_blocks.mutex_blocks));
	log_trace(logger,"CONSUMIR - MUTEX_BLOCKS - BLOCKED");
	pthread_mutex_unlock(&(archivo->mutex_file));

}


