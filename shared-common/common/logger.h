#ifndef COMMON_LOGGER_H_
#define COMMON_LOGGER_H_

#include <commons/log.h>

t_log* logger_create(char* file, char* program_name);

void logger_destroy(t_log* logger);

void logger_print_header(t_log* logger, char* program_name);

void logger_print_footer(t_log* logger, char* program_name);

#endif /* COMMON_LOGGER_H_ */
