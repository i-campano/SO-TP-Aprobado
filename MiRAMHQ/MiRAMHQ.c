/*
 *
 */

#include "MiRAMHQ.h"
int main(void)
{
	/*admin_memoria();
	return 0;
	*/
	crear_memoria_ppal();
	void iterator(char* value)
	{
		printf("%s\n", value);
	}

	logger = log_create("MiRAMHQ.log", "MiRAMHQ", 1, LOG_LEVEL_DEBUG);

	server_fd = iniciarServidor(5002);

	iniciarEstructurasAdministrativas();


	log_info(logger, "Servidor listo para recibir al clientexxxxx");
	manejadorDeHilos();

}
