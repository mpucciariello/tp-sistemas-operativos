#include "broker_logger.h"

int broker_logger_create(char* logfile_name)
{
	broker_log = logger_create(logfile_name, PROGRAM_NAME);
	if (broker_log == NULL || broker_log < 0)
	{
		perror("No ha sido posible instanciar el broker_logger");
		return -1;
	}

	logger_print_header(broker_log, PROGRAM_NAME);

	return 0;
}

void broker_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_info(broker_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void broker_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_warning(broker_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void broker_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_error(broker_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void broker_logger_destroy()
{
	logger_print_footer(broker_log, PROGRAM_NAME);
	logger_destroy(broker_log);
}

t_log* broker_log_get()
{
	return broker_log;
}
