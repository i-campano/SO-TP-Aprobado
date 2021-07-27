/*
 *
 */

#include "MiRAMHQ.h"
int main(void)
{

	crear_configuracion();
	void iterator(char* value)
	{
		printf("%s\n", value);
	}

	admin_memoria();
	server_fd = iniciarServidor(confDatos.puerto);
	iniciarEstructurasAdministrativas();
	if(mapaActivo){
		iniciar_mapa();
	}

	log_info(logger, "Servidor listo para recibir al clientexxxxx");
	manejadorDeHilos();
	/*config_destroy(configuracion);
	log_destroy(logger);
	free(mem_ppal);*/
}
