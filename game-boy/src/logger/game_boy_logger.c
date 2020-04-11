#include "game_boy_logger.h"

int game_boy_logger_create()
{
	game_boy_log = logger_create(LOG_FILE, PROGRAM_NAME);
	if (game_boy_log == NULL || game_boy_log < 0)
	{
		perror("No ha sido posible instanciar el game_boy_logger");
		return -1;
	}

	logger_print_header(game_boy_log, PROGRAM_NAME);

	return 0;
}

void game_boy_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_info(game_boy_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void game_boy_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_warning(game_boy_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void game_boy_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_error(game_boy_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void game_boy_logger_destroy()
{
	logger_print_footer(game_boy_log, PROGRAM_NAME);
	logger_destroy(game_boy_log);
}

t_log* game_boy_log_get()
{
	return game_boy_log;
}
