#ifndef GAME_BOY_H_
#define GAME_BOY_H_

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "config/game_boy_config.h"
#include "logger/game_boy_logger.h"
#include "../../shared-common/common/sockets.h"
#include "../../shared-common/common/utils.h"

int game_boy_broker_fd;
int game_boy_team_fd;
int game_boy_game_card_fd;

int game_boy_load();
void game_boy_init();
void game_boy_exit();

#endif /* GAME_BOY_H_ */
