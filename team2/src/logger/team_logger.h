#ifndef LOGGER_TEAM_LOGGER_H_
#define LOGGER_TEAM_LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../../shared-common/common/logger.h"

#define PROGRAM_NAME "TEAM"

int  team_logger_create(char* logfile_name);
void team_logger_info(char* message, ...);
void team_logger_warn(char* message, ...);
void team_logger_error(char* message, ...);
void team_logger_destroy();
t_log* team_log_get();

t_log* team_log;
#endif /* LOGGER_TEAM_LOGGER_H_ */
