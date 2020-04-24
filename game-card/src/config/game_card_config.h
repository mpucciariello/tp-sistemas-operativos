#ifndef CONFIG_game_card_CONFIG_H_
#define CONFIG_game_card_CONFIG_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>

#include "../logger/game_card_logger.h"

#include "../../../shared-common/common/config.h"

#define CONFIG_FILE_PATH "game-card.config"

typedef struct
{
	int tiempo_de_reintento_conexion;
	int tiempo_de_reintento_operacion;
	char* punto_montaje_tallgrass;
	char* ip_broker;
	int puerto_broker;
	char* ip_game_card;
	int puerto_game_card;
} t_game_card_config;

t_game_card_config* game_card_config;

int game_card_config_load();
void game_card_config_free();

#endif /* CONFIG_game_card_CONFIG_H_ */
