/*
 *
 */

#include "MiRAMHQ.h"
int main(void)
{
	logger = log_create("MiRAMHQ.log", "MiRAMHQ",0, LOG_LEVEL_DEBUG);
	void iterator(char* value)
	{
		printf("%s\n", value);
	}

	admin_memoria();
	server_fd = iniciarServidor(5002);
	iniciarEstructurasAdministrativas();
	iniciar_mapa();

	log_info(logger, "Servidor listo para recibir al clientexxxxx");
	manejadorDeHilos();

}
