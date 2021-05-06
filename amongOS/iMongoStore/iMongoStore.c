//MOngoStore :  este seria el FILESYSTEM del Sistema Operativo digamos?

// Funciones a implementar :
// Atender de forma paralela los diferentes tripulantes


//Iniciar FileSystem
//Generar las estructuras en memoria necesarias para poder
//contestar las peticiones del DISCODIADOR

/*
int create_block() {
	log_debug(logger, "create_block: void");

	int i;
	for (i = 0; i < bitmap->size * 8; i++) { /
		if (bitarray_test_bit(bitmap, i) == 0) {
			bitarray_set_bit(bitmap, i);
			FILE* f = fopen(string_from_format("%s/Bloques/%d.ims", mount_point, i), "w");
			if (f == NULL)
				return -1;

			fclose(f);
			update_bitmap_file();
			log_debug(logger, "create_block: int=%d", i);
			return i;
		}
	}

	log_debug(logger, "create_block: int=%d", -1);
	return -1;
}
*/


