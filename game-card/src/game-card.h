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

int game_card_fd;
bool is_connected;

int game_card_load();
void game_card_init();
void game_card_retry_connect(void* arg);
void game_card_init_as_server();
void *handle_connection(int fd);
void game_card_exit();
void subscribe_to(void *arg);

#endif /* GAME_CARD_H_ */
