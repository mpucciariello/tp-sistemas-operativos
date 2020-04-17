#include "game-boy.h"

int main(int argc, char *argv[])
{
	if(game_boy_load() < 0)
		return EXIT_FAILURE;
	game_boy_init();
	game_boy_exit();

	return EXIT_SUCCESS;
}

int game_boy_load()
{
	int response = game_boy_logger_create();
	if(response < 0)
		return response;

	response = game_boy_config_load();
	if(response < 0)
	{
		game_boy_logger_destroy();
		return response;
	}

	return 0;
}


void connect_to_broker()
{
	game_boy_broker_fd = socket_connect_to_server(game_boy_config->ip_broker, game_boy_config->puerto_broker);
	if(game_boy_broker_fd < 0)
	{
		game_boy_logger_warn("No se pudo conectar con BROKER");
		socket_close_conection(game_boy_broker_fd);
	}
	else
	{
		game_boy_logger_info("Conexion con BROKER establecida correctamente!");
	}
}

void connect_to_team()
{
	game_boy_team_fd = socket_connect_to_server(game_boy_config->ip_team, game_boy_config->puerto_team);
	if(game_boy_team_fd < 0)
	{
		game_boy_logger_warn("No se pudo conectar con TEAM");
		socket_close_conection(game_boy_team_fd);
	}
	else
	{
		game_boy_logger_info("Conexion con TEAM establecida correctamente!");
	}
}

void connect_to_game_card()
{
	game_boy_game_card_fd = socket_connect_to_server(game_boy_config->ip_gamecard, game_boy_config->puerto_gamecard);
	if (game_boy_game_card_fd < 0)
	{
		game_boy_logger_warn("No se pudo conectar con GAME CARD");
		socket_close_conection(game_boy_game_card_fd);
	}
	else
	{
		game_boy_logger_info("Conexion con GAME CARD establecida correctamente!");
	}
}

void game_boy_init()
{
	game_boy_logger_info("Inicando GAME BOY..");
	connect_to_broker();
	connect_to_team();
	connect_to_game_card();
}

void game_boy_exit()
{
	socket_close_conection(game_boy_broker_fd);
	socket_close_conection(game_boy_team_fd);
	socket_close_conection(game_boy_game_card_fd);
	game_boy_config_free();
	game_boy_logger_destroy();
}