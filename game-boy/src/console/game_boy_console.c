#include "game_boy_console.h"

char* game_boy_get_input();
char** game_boy_get_arguments_from_input(char*, int);
int game_boy_get_arguments_size(char *input);
void game_boy_command_execute(char *key, t_dictionary* command_actions, char** arguments, int arguments_size);

void broker_new_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 6) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("BROKER NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]");
		return;
	}
	game_boy_logger_info("BROKER NEW_POKEMON");
}

void broker_appeared_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 6) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("BROKER APPEARED_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]");
		return;
	}
	game_boy_logger_info("BROKER APPEARED_POKEMON");
}

void broker_catch_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 5) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("BROKER CATCH_POKEMON [POKEMON] [POSX] [POSY]");
		return;
	}
	game_boy_logger_info("BROKER CATCH_POKEMON");
}

void broker_caught_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 4) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("BROKER CAUGHT_POKEMON [ID_MENSAJE] [OK/FAIL]");
		return;
	}
	game_boy_logger_info("BROKER CAUGHT_POKEMON");
}

void broker_get_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 4) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("BROKER GET_POKEMON [POKEMON]");
		return;
	}
	game_boy_logger_info("BROKER GET_POKEMON");
}

void team_appeared_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 5) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("TEAM APPEARED_POKEMON [POKEMON] [POSX] [POSY]");
		return;
	}
	game_boy_logger_info("TEAM APPEARED_POKEMON");
}

void game_card_new_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 6) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("GAMECARD NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]");
		return;
	}
	game_boy_logger_info("GAMECARD NEW_POKEMON");
}

void game_card_catch_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 5) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("GAMECARD CATCH_POKEMON [POKEMON] [POSX] [POSY]");
		return;
	}
	game_boy_logger_info("GAMECARD CATCH_POKEMON");
}

void game_card_get_pokemon_command(char** arguments, int arguments_size) {
	if(arguments_size != 3) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("GAMECARD GET_POKEMON [POKEMON]");
		return;
	}
	game_boy_logger_info("GAMECARD GET_POKEMON");
}
void suscriptor_command(char** arguments, int arguments_size) {
	if(arguments_size != 3) {
		game_boy_logger_error("Comando o parametros invalidos");
		game_boy_logger_warn("SUSCRIPTOR [COLA_DE_MENSAJES] [TIEMPO]");
		return;
	}
	game_boy_logger_info("SUSCRIPTOR");
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
	if(!string_equals_ignore_case(concated_key, "SUSCRIPTOR")) {
		string_append(&concated_key, " ");
		string_append(&concated_key, arguments[1]);
	}

	char* key = concated_key;

	if (string_equals_ignore_case(key, EXIT_KEY))
		return -1;
	else
		game_boy_command_execute(key, command_actions, arguments, arguments_size);

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

void game_boy_command_execute(char *key, t_dictionary* command_actions, char** arguments, int arguments_size) {
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
