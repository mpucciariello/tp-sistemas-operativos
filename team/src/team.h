#ifndef TEAM_H_
#define TEAM_H_

#define LOCAL_IP "127.0.0.1"
#define LOCAL_PORT 5002

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include "config/team_config.h"
#include "logger/team_logger.h"
#include "planner/team_planner.h"
#include "../../shared-common/common/sockets.h"
#include "../../shared-common/common/utils.h"

int team_socket;
bool is_connected;

int team_load();
void team_init();
void team_server_init();
static void *handle_connection(void *arg);
void team_exit();
void *receive_msg(int broker_fd, int send_to);
void send_ack(void* arg);
void subscribe_to(void *arg);
void send_message_test();
void team_retry_connect(void* arg);

#endif /* TEAM_H_ */
