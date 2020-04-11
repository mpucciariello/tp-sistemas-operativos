#include "config.h"

int config_load(t_log* log, char* path, void (*read)(t_config*), void (*print)())
{
	if(path == NULL)
	{
		log_error(log, "No se ha ingresado la ruta del archivo de configuracion");
		return -1;
	}

	t_config *config_file = config_create(path);
	if(config_file == NULL || config_file < 0)
	{
		log_error(log, "No se pudo crear el archivo de configuracion");
		return -1;
	}
	read(config_file);
	config_destroy(config_file);

	if(log != NULL)
	{
		print();
	}

	return 0;
}
