#ifndef CONFIG_GAME_BOY_CONFIG_H_
#define CONFIG_GAME_BOY_CONFIG_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>

#include "../../../shared-common/common/config.h"

#include "../logger/game_boy_logger.h"

#define CONFIG_FILE_PATH "game-boy.config"

typedef struct
{
	char* ip_broker;
	char* ip_team;
	char* ip_gamecard;
	int puerto_broker;
	int puerto_team;
	int puerto_gamecard;

} t_game_boy_config;

t_game_boy_config* game_boy_config;

int game_boy_config_load();
void game_boy_config_free();

#endif /* CONFIG_GAME_BOY_CONFIG_H_ */
