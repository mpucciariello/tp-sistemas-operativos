#ifndef GAME_CARD_H_
#define GAME_CARD_H_

#define LOCAL_IP "127.0.0.1"
#define LOCAL_PORT 5001

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "config/game_card_config.h"
#include "logger/game_card_logger.h"
#include "file_system/game_card_file_system.h"
#include "../../shared-common/common/sockets.h"
#include "../../shared-common/common/utils.h"


int game_card_fd, game_card_socket;

int game_card_load();
void game_card_and_broker_connection_init();
void game_card_and_gameboy_connection_init();
void game_card_exit();
void subscribe_to(void *arg);

#endif /* GAME_CARD_H_ */
