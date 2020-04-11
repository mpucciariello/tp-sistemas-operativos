#ifndef COMMON_CONFIG_H_
#define COMMON_CONFIG_H_

#include <stdio.h>
#include <commons/config.h>
#include <commons/log.h>

int config_load(t_log* log, char* path, void (*read)(t_config*), void (*print)());

#endif /* COMMON_CONFIG_H_ */
