#include "game_boy_config.h"

void read_config(t_config* config_file);
void print_config();

int game_boy_config_load()
{
	//game_boy_logger_info("Se establecerá la configuración");
	return config_load(game_boy_log_get(), CONFIG_FILE_PATH, read_config, print_config);
}

void game_boy_config_free()
{
	free(game_boy_config->ip_broker);
	free(game_boy_config->ip_team);
	free(game_boy_config->ip_gamecard);
	free(game_boy_config);
}

void read_config(t_config* config_file)
{
	game_boy_config = malloc(sizeof(t_game_boy_config));
	game_boy_config->ip_broker = malloc(sizeof(char*));
	game_boy_config->ip_broker = string_duplicate(config_get_string_value(config_file, "IP_BROKER"));
	game_boy_config->ip_team = malloc(sizeof(char*));
	game_boy_config->ip_team = string_duplicate(config_get_string_value(config_file, "IP_TEAM"));
	game_boy_config->ip_gamecard = malloc(sizeof(char*));
	game_boy_config->ip_gamecard = string_duplicate(config_get_string_value(config_file, "IP_GAMECARD"));
	game_boy_config->puerto_broker = config_get_int_value(config_file, "PUERTO_BROKER");
	game_boy_config->puerto_team = config_get_int_value(config_file, "PUERTO_TEAM");
	game_boy_config->puerto_gamecard = config_get_int_value(config_file, "PUERTO_GAMECARD");
}

void print_config()
{
	//game_boy_logger_info("IP_BROKER: %s", game_boy_config->ip_broker);
	//game_boy_logger_info("IP_TEAM: %s", game_boy_config->ip_team);
	//game_boy_logger_info("IP_GAMECARD: %s", game_boy_config->ip_gamecard);
	//game_boy_logger_info("PUERTO_BROKER: %d", game_boy_config->puerto_broker);
	//game_boy_logger_info("PUERTO_TEAM: %d", game_boy_config->puerto_team);
	//game_boy_logger_info("PUERTO_GAMECARD: %d", game_boy_config->puerto_gamecard);
}
