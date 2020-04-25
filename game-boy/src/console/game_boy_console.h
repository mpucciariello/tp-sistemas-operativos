#ifndef CONSOLE_GAME_BOY_CONSOLE_H_
#define CONSOLE_GAME_BOY_CONSOLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "../logger/game_boy_logger.h"

#define BROKER_NEW "BROKER NEW_POKEMON"
#define BROKER_APPEARED "BROKER APPEARED_POKEMON"
#define BROKER_CATCH "BROKER CATCH_POKEMON"
#define BROKER_CAUGHT "BROKER CAUGHT_POKEMON"
#define BROKER_GET "BROKER GET_POKEMON"
#define TEAM_APPEARED "TEAM APPEARED_POKEMON"
#define GAMECARD_NEW "GAMECARD NEW_POKEMON"
#define GAMECARD_CATCH "GAMECARD CATCH_POKEMON"
#define GAMECARD_GET "GAMECARD GET_POKEMON"
#define SUSCRIPTOR "SUSCRIPTOR"
#define EXIT_KEY "EXIT"
#define SPLIT_CHAR " "
#define INPUT_SIZE 100

typedef struct {
	char *key;
	void (*action)(char**, int);
} t_command;

int game_boy_console_read(t_dictionary*);
t_dictionary* game_boy_get_command_actions();
void game_boy_free_command_actions(t_dictionary*);

#endif /* CONSOLE_GAME_BOY_CONSOLE_H_ */
