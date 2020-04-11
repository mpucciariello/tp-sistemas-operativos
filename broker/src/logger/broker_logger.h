#ifndef LOGGER_BROKER_LOGGER_H_
#define LOGGER_BROKER_LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../../shared-common/common/logger.h"

#define PROGRAM_NAME "BROKER"

int  broker_logger_create(char* logfile_name);
void broker_logger_info(char* message, ...);
void broker_logger_warn(char* message, ...);
void broker_logger_error(char* message, ...);
void broker_logger_destroy();
t_log* broker_log_get();

t_log* broker_log;
#endif /* LOGGER_BROKER_LOGGER_H_ */
