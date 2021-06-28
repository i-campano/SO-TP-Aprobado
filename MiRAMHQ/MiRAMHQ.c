/*
 *
 */

#include "MiRAMHQ.h"
int main(void)
{
	logger = log_create("MiRAMHQ.log", "MiRAMHQ", 1, LOG_LEVEL_DEBUG);
	crear_memoria_ppal();
	log_info(logger,"%i",memoria_libre());
	log_info(logger,"Hola");
	void iterator(char* value)
	{
		printf("%s\n", value);
	}



	server_fd = iniciarServidor(5002);

	iniciarEstructurasAdministrativas();


	log_info(logger, "Servidor listo para recibir al clientexxxxx");
	manejadorDeHilos();

}
