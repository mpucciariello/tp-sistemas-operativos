#include "game-boy.h"

int main(int argc, char *argv[]) {

	if (game_boy_load() < 0)
		return EXIT_FAILURE;
	if (argc > 3) {
		game_boy_init(argc, argv);
		game_boy_exit();
	} else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int game_boy_load() {
	int response = game_boy_logger_create();
	if (response < 0)
		return response;

	response = game_boy_config_load();
	if (response < 0) {
		game_boy_logger_destroy();
		return response;
	}

	return 0;
}

void connect_to_broker() {
	game_boy_broker_fd = socket_connect_to_server(game_boy_config->ip_broker,
			game_boy_config->puerto_broker);
	if (game_boy_broker_fd < 0) {
		socket_close_conection(game_boy_broker_fd);
		exit(EXIT_FAILURE);
	} else {
		game_boy_logger_info("Conexion con BROKER establecida correctamente!");
		connected = true;
	}
}

void connect_to_team() {
	game_boy_team_fd = socket_connect_to_server(game_boy_config->ip_team,
			game_boy_config->puerto_team);
	if (game_boy_team_fd < 0) {
		socket_close_conection(game_boy_team_fd);
	} else {
		game_boy_logger_info("Conexion con TEAM establecida correctamente!");
		connected = true;
	}
}

void connect_to_game_card() {
	game_boy_game_card_fd = socket_connect_to_server(
			game_boy_config->ip_gamecard, game_boy_config->puerto_gamecard);
	if (game_boy_game_card_fd < 0) {
		socket_close_conection(game_boy_game_card_fd);
	} else {
		game_boy_logger_info(
				"Conexion con GAME CARD establecida correctamente!");
		connected = true;
	}
}

void game_boy_console(void* _args_) {
	struct t_console_args* args = _args_;

	command_actions = game_boy_get_command_actions();
	game_boy_console_read(command_actions, args->arguments, args->argcount);
}

void game_boy_init(int argcount, char* arguments[]) {
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

	char* option = arguments[1];
	char* match = "";
	int arraylen = sizeof(valid_options) / sizeof(valid_options[0]);
	for (int i = 0; i < arraylen; i++) {
		if (strcmp(valid_options[i], option) == 0) {
			match = option;
		}
	}

	if (string_is_empty(match)) {
		exit(EXIT_FAILURE);
	}

	else {
		if (string_equals_ignore_case(match, "TEAM")) {
			game_boy_logger_info("Creando un hilo para enviar al team");
			pthread_create(&tid[0], NULL, (void*) connect_to_team, NULL);
		}

		if (string_equals_ignore_case(match, "GAMECARD")) {
			game_boy_logger_info("Creando un hilo para enviar al game card");
			pthread_create(&tid[0], NULL, (void*) connect_to_game_card, NULL);
		}

		if (string_equals_ignore_case(match, "BROKER")
				|| string_equals_ignore_case(match, "SUBSCRIBE")) {
			game_boy_logger_info("Creando un hilo para enviar al broker");
			pthread_create(&tid[0], NULL, (void*) connect_to_broker, NULL);
		}

		struct t_console_args args;
		int arraylen = argcount;
		int argsarrsize = sizeof(args.arguments) / sizeof(args.arguments[0]);

		for (int i = 0; i < arraylen; i++) {
			args.arguments[i] = arguments[i];
		}

		int to_pad = argsarrsize - arraylen;
		int from = argsarrsize - to_pad;
		int to = argsarrsize - 1;

		for (int i = from; i <= to; i++) {
			args.arguments[i] = 0;
		}

		args.argcount = argcount;

		while(!connected){
			sleep(1);
		}

		game_boy_logger_info("Creando un hilo para consola");
		pthread_create(&tid[1], NULL, (void*) game_boy_console, &args);
		for(int i=0; i<2; i++) {
			pthread_join(tid[i], NULL);
		}

		game_boy_logger_info("All threads finished");
	}
}

void game_boy_exit() {
	socket_close_conection(game_boy_broker_fd);
	socket_close_conection(game_boy_team_fd);
	socket_close_conection(game_boy_game_card_fd);
	game_boy_free_command_actions(command_actions);
	game_boy_config_free();
	game_boy_logger_destroy();
}
