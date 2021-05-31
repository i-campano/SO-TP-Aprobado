/*
 *
 */

#include "MiRAMHQ.h"
#include "socket.h"

int main(void)
{
	void iterator(char* value)
	{
		printf("%s\n", value);
	}

	logger = log_create("MiRAMHQ.log", "MiRAMHQ", 1, LOG_LEVEL_DEBUG);

	server_fd = iniciarServidor(5002);
	log_info(logger, "Servidor listo para recibir al clientexxxxx");
	manejadorDeHilos();


}
