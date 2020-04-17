#ifndef LOGGER_GAME_CARD_LOGGER_H_
#define LOGGER_GAME_CARD_LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../../shared-common/common/logger.h"

#define LOG_FILE "/home/utnso/log_gamecard.txt"
#define PROGRAM_NAME "GAME_CARD"

int  game_card_logger_create();
void game_card_logger_info(char* message, ...);
void game_card_logger_warn(char* message, ...);
void game_card_logger_error(char* message, ...);
void game_card_logger_destroy();
t_log* game_card_log_get();

t_log* game_card_log;
#endif /* LOGGER_GAME_CARD_LOGGER_H_ */
