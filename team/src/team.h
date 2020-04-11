#ifndef TEAM_H_
#define TEAM_H_

#define LOCAL_IP "127.0.0.1"
#define LOCAL_PORT 5002

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "config/team_config.h"
#include "logger/team_logger.h"
#include "planner/team_planner.h"
#include "../../shared-common/common/sockets.h"
#include "../../shared-common/common/utils.h"

int team_fd, team_socket;

int team_load();
void team_init();
void team_server_init();
static void *handle_connection(void *arg);
void team_exit();

#endif /* TEAM_H_ */
