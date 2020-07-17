#include "logger.h"

#include <stdbool.h>

t_log* logger_create(char* file, char* program_name)
{
	return log_create(file, program_name, false, LOG_LEVEL_TRACE);
}

void logger_destroy(t_log* logger)
{
	log_destroy(logger);
}

void logger_print_header(t_log* logger, char* program_name)
{
	log_info(logger,
			"\n\t\e[31;1m===========================================\e[0m\n"
					"\t.::	Bievenido a Delibird - %s	::."
					"\n\t\e[31;1m===========================================\e[0m",
			program_name);
}

void logger_print_footer(t_log* logger, char* program_name)
{
	log_info(logger,
			"\n\t\e[31;1m============================================================\e[0m\n"
					"\t.::	Gracias por utilizar Delibird, %s terminada.	::."
					"\n\t\e[31;1m============================================================\e[0m",
			program_name);
}
