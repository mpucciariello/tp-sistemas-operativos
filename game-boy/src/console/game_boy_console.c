#include "game_boy_console.h"

char* game_boy_get_input();
char** game_boy_get_arguments_from_input(char*, int);
int game_boy_get_arguments_size(char *input);
void game_boy_command_execute(char *key, t_dictionary* command_actions,
		char** arguments, int arguments_size);

void broker_new_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 6) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn(
				"BROKER NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]");
		return;
	}
	game_boy_logger_info("BROKER NEW_POKEMON");
	t_new_pokemon* new_snd = malloc(sizeof(t_new_pokemon));
	new_snd->nombre_pokemon = string_duplicate(arguments[2]);
	new_snd->tamanio_nombre = strlen(arguments[2]);
	new_snd->pos_x = atoi(arguments[3]);
	new_snd->pos_y = atoi(arguments[4]);
	new_snd->cantidad = atoi(arguments[5]);

	utils_serialize_and_send(game_boy_broker_fd, NEW_POKEMON, new_snd);
	game_boy_logger_info("Envio de NEW Pokemon");
}

void broker_appeared_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 6) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn(
				"BROKER APPEARED_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]");
		return;
	}
	game_boy_logger_info("BROKER APPEARED_POKEMON");
	t_appeared_pokemon* appeared_snd = malloc(sizeof(t_appeared_pokemon));
	appeared_snd->nombre_pokemon = string_duplicate(arguments[2]);
	appeared_snd->tamanio_nombre = strlen(arguments[2]);
	appeared_snd->pos_x = atoi(arguments[3]);
	appeared_snd->pos_y = atoi(arguments[4]);
	appeared_snd->id_correlacional = atoi(arguments[5]);

	utils_serialize_and_send(game_boy_broker_fd, APPEARED_POKEMON,
			appeared_snd);
	game_boy_logger_info("Envio de APPEARED Pokemon");
}

void broker_catch_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 5) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("BROKER CATCH_POKEMON [POKEMON] [POSX] [POSY]");
		return;
	}
	game_boy_logger_info("BROKER CATCH_POKEMON");
	t_catch_pokemon* catch_snd = malloc(sizeof(t_catch_pokemon));
	catch_snd->nombre_pokemon = string_duplicate(arguments[2]);
	catch_snd->tamanio_nombre = strlen(arguments[2]);
	catch_snd->pos_x = atoi(arguments[3]);
	catch_snd->pos_y = atoi(arguments[4]);

	utils_serialize_and_send(game_boy_broker_fd, CATCH_POKEMON, catch_snd);
	game_boy_logger_info("Envio de CATCH_POKEMON Pokemon");
}

void broker_caught_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 4) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("BROKER CAUGHT_POKEMON [ID_MENSAJE] [OK/FAIL]");
		return;
	}
	game_boy_logger_info("BROKER CAUGHT_POKEMON");
	t_caught_pokemon* caught_snd = malloc(sizeof(t_caught_pokemon));
	caught_snd->id_correlacional = atoi(arguments[2]);
	char* ok_fail = string_duplicate(arguments[3]);
	int result = 0;
	if (string_equals_ignore_case(ok_fail, "fail")) {
		result = 1;
	}
	caught_snd->result = result;

	utils_serialize_and_send(game_boy_broker_fd, CAUGHT_POKEMON, caught_snd);
	game_boy_logger_info("Envio de CAUGHT_POKEMON Pokemon");
}

void broker_get_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 4) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("BROKER GET_POKEMON [POKEMON]");
		return;
	}
	game_boy_logger_info("BROKER GET_POKEMON");
	t_get_pokemon* get_snd = malloc(sizeof(t_get_pokemon));
	get_snd->nombre_pokemon = string_duplicate(arguments[2]);
	get_snd->tamanio_nombre = strlen(arguments[2]);

	utils_serialize_and_send(game_boy_broker_fd, GET_POKEMON, get_snd);
	game_boy_logger_info("Envio de GET_POKEMON Pokemon");
}

void team_appeared_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 5) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("TEAM APPEARED_POKEMON [POKEMON] [POSX] [POSY]");
		return;
	}
	game_boy_logger_info("TEAM APPEARED_POKEMON");
	t_appeared_pokemon* appeared_snd = malloc(sizeof(t_appeared_pokemon));
	appeared_snd->nombre_pokemon = string_duplicate(arguments[2]);
	appeared_snd->tamanio_nombre = strlen(arguments[2]);
	appeared_snd->pos_x = atoi(arguments[3]);
	appeared_snd->pos_y = atoi(arguments[4]);

	utils_serialize_and_send(game_boy_team_fd, APPEARED_POKEMON, appeared_snd);
	game_boy_logger_info("Envio de APPEARED Pokemon");
}

void game_card_new_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 7) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn(
				"GAMECARD NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD] [ID_MENSAJE]");
		return;
	}
	game_boy_logger_info("GAMECARD NEW_POKEMON");
	t_new_pokemon* new_snd = malloc(sizeof(t_new_pokemon));
	new_snd->nombre_pokemon = string_duplicate(arguments[2]);
	new_snd->tamanio_nombre = strlen(arguments[2]);
	new_snd->pos_x = atoi(arguments[3]);
	new_snd->pos_y = atoi(arguments[4]);
	new_snd->cantidad = atoi(arguments[5]);
	new_snd->id_correlacional = atoi(arguments[6]);

	utils_serialize_and_send(game_boy_game_card_fd, NEW_POKEMON, new_snd);
	game_boy_logger_info("Envio de NEW Pokemon");
}

void game_card_catch_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 6) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn(
				"GAMECARD CATCH_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]");
		return;
	}
	game_boy_logger_info("GAMECARD CATCH_POKEMON");
	t_catch_pokemon* catch_snd = malloc(sizeof(t_catch_pokemon));
	catch_snd->nombre_pokemon = string_duplicate(arguments[2]);
	catch_snd->tamanio_nombre = strlen(arguments[2]);
	catch_snd->pos_x = atoi(arguments[3]);
	catch_snd->pos_y = atoi(arguments[4]);
	catch_snd->id_correlacional = atoi(arguments[5]);

	utils_serialize_and_send(game_boy_game_card_fd, CATCH_POKEMON, catch_snd);
	game_boy_logger_info("Envio de CATCH_POKEMON Pokemon");
}

void game_card_get_pokemon_command(char** arguments, int arguments_size) {
	if (arguments_size != 3) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("GAMECARD GET_POKEMON [POKEMON]");
		return;
	}
	game_boy_logger_info("GAMECARD GET_POKEMON");
	t_get_pokemon* get_snd = malloc(sizeof(t_get_pokemon));
	get_snd->nombre_pokemon = string_duplicate(arguments[2]);
	get_snd->tamanio_nombre = strlen(arguments[2]);

	utils_serialize_and_send(game_boy_game_card_fd, GET_POKEMON, get_snd);
	game_boy_logger_info("Envio de GET_POKEMON Pokemon");
}

t_cola get_queue_by_name(char* cola) {
	if (string_equals_ignore_case(cola, "new_queue")) {
		return NEW_QUEUE;
	} else if (string_equals_ignore_case(cola, "appeared_queue")) {
		return APPEARED_QUEUE;
	} else if (string_equals_ignore_case(cola, "localized_queue")) {
		return LOCALIZED_QUEUE;
	} else if (string_equals_ignore_case(cola, "get_queue")) {
		return GET_QUEUE;
	} else if (string_equals_ignore_case(cola, "catch_queue")) {
		return CATCH_QUEUE;
	} else if (string_equals_ignore_case(cola, "caught_queue")) {
		return CAUGHT_QUEUE;
	} else {
		return -1;
	}
}

void suscriptor_command(char** arguments, int arguments_size) {
	if (arguments_size != 3) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("SUSCRIPTOR [COLA_DE_MENSAJES] [TIEMPO]");
		return;
	}

	game_boy_logger_info("SUSCRIPTOR");
	t_subscribe* sub_snd = malloc(sizeof(t_subscribe));
	t_protocol subscribe_protocol = SUBSCRIBE;
	sub_snd->proceso = GAME_BOY;
	sub_snd->ip = "GB";
	sub_snd->puerto = 0;
	sub_snd->cola = get_queue_by_name(arguments[1]);
	sub_snd->seconds = atoi(arguments[2]);
	utils_serialize_and_send(game_boy_broker_fd, subscribe_protocol, sub_snd);
	game_boy_logger_info("Envio de SUBSCRIBE");

	int protocol;
	int received_bytes;

	while (true) {
		received_bytes = recv(game_boy_broker_fd, &protocol, sizeof(int), 0);

		if (received_bytes <= 0) {
			game_boy_logger_error("Error al recibir mensaje");
		}

		switch (protocol) {

		case NEW_POKEMON: {
			game_boy_logger_info("NEW received");
			t_new_pokemon *new_receive = utils_receive_and_deserialize(
					game_boy_broker_fd, protocol);
			game_boy_logger_info("ID recibido: %d", new_receive->id);
			game_boy_logger_info("ID Correlacional: %d",
					new_receive->id_correlacional);
			game_boy_logger_info("Cantidad: %d", new_receive->cantidad);
			game_boy_logger_info("Nombre Pokemon: %s",
					new_receive->nombre_pokemon);
			game_boy_logger_info("Largo Nombre: %d",
					new_receive->tamanio_nombre);
			game_boy_logger_info("Posicion X: %d", new_receive->pos_x);
			game_boy_logger_info("Posicion Y: %d", new_receive->pos_y);

			usleep(100000);
			break;
		}

		case GET_POKEMON: {
			game_boy_logger_info("GET received");
			t_get_pokemon *get_rcv = utils_receive_and_deserialize(
					game_boy_broker_fd, protocol);
			game_boy_logger_info("ID correlacional: %d",
					get_rcv->id_correlacional);
			game_boy_logger_info("Nombre Pokemon: %s", get_rcv->nombre_pokemon);
			game_boy_logger_info("Largo nombre: %d", get_rcv->tamanio_nombre);

			usleep(50000);
			break;
		}

		case CATCH_POKEMON: {
			game_boy_logger_info("CATCH received");
			t_catch_pokemon *catch_rcv = utils_receive_and_deserialize(
					game_boy_broker_fd, protocol);
			game_boy_logger_info("ID correlacional: %d",
					catch_rcv->id_correlacional);
			game_boy_logger_info("ID Generado: %d", catch_rcv->id_gen);
			game_boy_logger_info("Nombre Pokemon: %s",
					catch_rcv->nombre_pokemon);
			game_boy_logger_info("Largo nombre: %d", catch_rcv->tamanio_nombre);
			game_boy_logger_info("Posicion X: %d", catch_rcv->pos_x);
			game_boy_logger_info("Posicion Y: %d", catch_rcv->pos_y);

			usleep(50000);
			break;
		}

		case CAUGHT_POKEMON: {
			game_boy_logger_info("Caught received");
			t_caught_pokemon *caught_rcv = utils_receive_and_deserialize(game_boy_broker_fd,
					protocol);
			game_boy_logger_info("ID correlacional: %d",
					caught_rcv->id_correlacional);
			game_boy_logger_info("ID mensaje: %d", caught_rcv->id_msg);
			game_boy_logger_info("Resultado (0/1): %d", caught_rcv->result);
			usleep(50000);
			break;
		}

		case LOCALIZED_POKEMON: {
			game_boy_logger_info("Localized received");
			t_localized_pokemon *loc_rcv = utils_receive_and_deserialize(game_boy_broker_fd,
					protocol);
			game_boy_logger_info("ID correlacional: %d",

			loc_rcv->id_correlacional);
			game_boy_logger_info("Nombre Pokemon: %s", loc_rcv->nombre_pokemon);
			game_boy_logger_info("Largo nombre: %d", loc_rcv->tamanio_nombre);
			game_boy_logger_info("Cant Elementos en lista: %d",
					loc_rcv->cant_elem);
			for (int el = 0; el < loc_rcv->cant_elem; el++) {
				t_position* pos = malloc(sizeof(t_position));
				pos = list_get(loc_rcv->posiciones, el);
				game_boy_logger_info("Position is (%d, %d)", pos->pos_x,
						pos->pos_y);
			}
			usleep(500000);
			break;
		}

		case APPEARED_POKEMON: {
			game_boy_logger_info("Appeared received");
			t_appeared_pokemon *appeared_rcv = utils_receive_and_deserialize(game_boy_broker_fd,
					protocol);
			game_boy_logger_info("ID correlacional: %d",
					appeared_rcv->id_correlacional);
			game_boy_logger_info("Cantidad: %d", appeared_rcv->cantidad);
			game_boy_logger_info("Nombre Pokemon: %s",
					appeared_rcv->nombre_pokemon);
			game_boy_logger_info("Largo nombre: %d",
					appeared_rcv->tamanio_nombre);
			game_boy_logger_info("Posicion X: %d", appeared_rcv->pos_x);
			game_boy_logger_info("Posicion Y: %d", appeared_rcv->pos_y);

			usleep(50000);
			break;
		}

		default:
			break;
		}
	}

}

int game_boy_console_read(t_dictionary* command_actions) {
	char* input = game_boy_get_input();
	if (input == NULL)
		return 0;

	string_to_upper(input);
	int arguments_size = game_boy_get_arguments_size(input);

	char** arguments = game_boy_get_arguments_from_input(input, arguments_size);

	char *concated_key = string_new();
	string_append(&concated_key, arguments[0]);
	if (!string_equals_ignore_case(concated_key, "SUSCRIPTOR")) {
		string_append(&concated_key, " ");
		string_append(&concated_key, arguments[1]);
	}

	char* key = concated_key;

	if (string_equals_ignore_case(key, EXIT_KEY))
		return -1;
	else
		game_boy_command_execute(key, command_actions, arguments,
				arguments_size);

	free(arguments);
	free(input);

	return 0;
}

char* game_boy_get_input() {
	char *input = malloc(INPUT_SIZE);
	fgets(input, INPUT_SIZE, stdin);

	string_trim(&input);

	if (string_is_empty(input)) {
		free(input);
		return NULL;
	}

	return input;
}

int game_boy_get_arguments_size(char *input) {
	char last_char = 'a';
	int count = 1;
	for (int i = 0; i < string_length(input); i++) {
		if (i != 0)
			last_char = input[i - 1];
		if (input[i] == ' ' && last_char != ' ')
			count++;
	}

	return count;
}

char** game_boy_get_arguments_from_input(char* input, int arguments_size) {
	char** arguments = malloc(sizeof(char*) * arguments_size);

	if (string_contains(input, SPLIT_CHAR))
		arguments = string_split(input, SPLIT_CHAR);
	else
		arguments[0] = string_duplicate(input);

	return arguments;
}

void game_boy_command_execute(char *key, t_dictionary* command_actions,
		char** arguments, int arguments_size) {
	t_command* command = dictionary_get(command_actions, key);
	if (command == NULL)
		game_boy_logger_warn("Comando %s desconocido", key);
	else
		command->action(arguments, arguments_size);
}

t_dictionary* game_boy_get_command_actions() {
	t_dictionary* command_actions = dictionary_create();

	t_command* broker_new_command = malloc(sizeof(t_command));
	broker_new_command->key = BROKER_NEW;
	broker_new_command->action = broker_new_pokemon_command;
	dictionary_put(command_actions, BROKER_NEW, broker_new_command);

	t_command* broker_appeared_command = malloc(sizeof(t_command));
	broker_appeared_command->key = BROKER_APPEARED;
	broker_appeared_command->action = broker_appeared_pokemon_command;
	dictionary_put(command_actions, BROKER_APPEARED, broker_appeared_command);

	t_command* broker_catch_command = malloc(sizeof(t_command));
	broker_catch_command->key = BROKER_CATCH;
	broker_catch_command->action = broker_catch_pokemon_command;
	dictionary_put(command_actions, BROKER_CATCH, broker_catch_command);

	t_command* broker_caught_command = malloc(sizeof(t_command));
	broker_caught_command->key = BROKER_CAUGHT;
	broker_caught_command->action = broker_caught_pokemon_command;
	dictionary_put(command_actions, BROKER_CAUGHT, broker_caught_command);

	t_command* broker_get_command = malloc(sizeof(t_command));
	broker_get_command->key = BROKER_GET;
	broker_get_command->action = broker_get_pokemon_command;
	dictionary_put(command_actions, BROKER_GET, broker_get_command);

	t_command* team_appeared_command = malloc(sizeof(t_command));
	team_appeared_command->key = TEAM_APPEARED;
	team_appeared_command->action = team_appeared_pokemon_command;
	dictionary_put(command_actions, TEAM_APPEARED, team_appeared_command);

	t_command* game_card_new_command = malloc(sizeof(t_command));
	game_card_new_command->key = GAMECARD_NEW;
	game_card_new_command->action = game_card_new_pokemon_command;
	dictionary_put(command_actions, GAMECARD_NEW, game_card_new_command);

	t_command* game_card_catch_command = malloc(sizeof(t_command));
	game_card_catch_command->key = GAMECARD_CATCH;
	game_card_catch_command->action = game_card_catch_pokemon_command;
	dictionary_put(command_actions, GAMECARD_CATCH, game_card_catch_command);

	t_command* game_card_get_command = malloc(sizeof(t_command));
	game_card_get_command->key = GAMECARD_GET;
	game_card_get_command->action = game_card_get_pokemon_command;
	dictionary_put(command_actions, GAMECARD_GET, game_card_get_command);

	t_command* suscript_command = malloc(sizeof(t_command));
	suscript_command->key = SUSCRIPTOR;
	suscript_command->action = suscriptor_command;
	dictionary_put(command_actions, SUSCRIPTOR, suscript_command);

	return command_actions;
}

void command_destroyer(void* command) {
	free(command);
}

void game_boy_free_command_actions(t_dictionary* command_actions) {
	dictionary_destroy_and_destroy_elements(command_actions, command_destroyer);
}
