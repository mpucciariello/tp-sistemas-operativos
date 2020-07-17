#include "team_logger.h"

int team_logger_create(char* logfile_name)
{
	team_log = logger_create(logfile_name, PROGRAM_NAME);
	if (team_log == NULL || team_log < 0)
	{
		perror("No ha sido posible instanciar el team_logger.");
		return -1;
	}

	logger_print_header(team_log, PROGRAM_NAME);

	return 0;
}

void team_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_info(team_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void team_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_warning(team_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void team_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_error(team_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void team_logger_destroy()
{
	logger_print_footer(team_log, PROGRAM_NAME);
	logger_destroy(team_log);
}

t_log* team_log_get()
{
	return team_log;
}
