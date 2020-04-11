#include "game_card_logger.h"

int game_card_logger_create()
{
	game_card_log = logger_create(LOG_FILE, PROGRAM_NAME);
	if (game_card_log == NULL || game_card_log < 0)
	{
		perror("No ha sido posible instanciar el game_card_logger");
		return -1;
	}

	logger_print_header(game_card_log, PROGRAM_NAME);

	return 0;
}

void game_card_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_info(game_card_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void game_card_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_warning(game_card_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void game_card_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_error(game_card_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void game_card_logger_destroy()
{
	logger_print_footer(game_card_log, PROGRAM_NAME);
	logger_destroy(game_card_log);
}

t_log* game_card_log_get()
{
	return game_card_log;
}
