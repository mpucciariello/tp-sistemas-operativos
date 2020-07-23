#include "game_card_config.h"

void read_config(t_config* config_file);
void print_config();

int game_card_config_load()
{
	game_card_logger_info("Se establecerá la configuración");
	return config_load(game_card_log_get(), CONFIG_FILE_PATH, read_config, print_config);
}

void game_card_config_free()
{
	free(game_card_config->punto_montaje_tallgrass);
	free(game_card_config->ip_broker);
	free(game_card_config);
}

void read_config(t_config* config_file)
{
	game_card_config = malloc(sizeof(t_game_card_config));
	game_card_config->tiempo_de_reintento_conexion = config_get_int_value(config_file, "TIEMPO_DE_REINTENTO_CONEXION");
	game_card_config->tiempo_de_reintento_operacion = config_get_int_value(config_file, "TIEMPO_DE_REINTENTO_OPERACION");
	game_card_config->punto_montaje_tallgrass = malloc(sizeof(char*));
	game_card_config->punto_montaje_tallgrass = string_duplicate(config_get_string_value(config_file, "PUNTO_MONTAJE_TALLGRASS"));
	game_card_config->ip_broker = string_duplicate(config_get_string_value(config_file, "IP_BROKER"));
	game_card_config->puerto_broker = config_get_int_value(config_file, "PUERTO_BROKER");
	game_card_config->ip_game_card = string_duplicate(config_get_string_value(config_file, "IP_GAMECARD"));
	game_card_config->puerto_game_card = config_get_int_value(config_file, "PUERTO_GAMECARD");

}

void print_config()
{
	game_card_logger_info("TIEMPO_DE_REINTENTO_CONEXION: %d", game_card_config->tiempo_de_reintento_conexion);
	game_card_logger_info("TIEMPO_DE_REINTENTO_OPERACION: %d", game_card_config->tiempo_de_reintento_operacion);
	game_card_logger_info("PUNTO_MONTAJE_TALLGRASS: %s", game_card_config->punto_montaje_tallgrass);
	game_card_logger_info("IP_BROKER: %s", game_card_config->ip_broker);
	game_card_logger_info("PUERTO_BROKER: %d", game_card_config->puerto_broker);
	game_card_logger_info("IP_GAMECARD: %s", game_card_config->ip_game_card);
	game_card_logger_info("PUERTO_GAMECARD: %d", game_card_config->puerto_game_card);
}
