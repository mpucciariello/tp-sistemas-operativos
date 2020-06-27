#ifndef TEAM_H_
#define TEAM_H_

#define LOCAL_IP "127.0.0.1"
#define LOCAL_PORT 5002

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include "config/team_config.h"
#include "logger/team_logger.h"
#include "planner/team_planner.h"
#include "../../shared-common/common/sockets.h"
#include "../../shared-common/common/utils.h"

int team_socket;
bool is_connected;
pthread_t planificator;
pthread_t algoritmo_cercania_entrenadores;
sem_t sem_message_on_queue;
t_list* get_id_corr;
t_list* keys_list;
t_list* message_catch_sended;


int team_load();
void team_init();
void team_server_init();
void *handle_connection(void *arg);
void team_exit();
void *receive_msg(int broker_fd, int send_to);
void send_ack(void* arg);
void subscribe_to(void *arg);
void send_message_test();
void team_retry_connect(void* arg);
bool pokemon_required(char* name);
void send_get_message();
void move_trainers_and_catch_pokemon(t_entrenador_pokemon*);
int send_message(void* paquete, t_protocol protocolo, t_list* queue);
bool trainer_completed_with_success(t_entrenador_pokemon*);
void delete_from_bloqued_queue(t_entrenador_pokemon*, int);
bool trainer_is_in_deadlock_caught(t_entrenador_pokemon*);
void send_message_catch(t_catch_pokemon*);
void check_SJF_CD_time(t_entrenador_pokemon*);
void check_RR_burst(t_entrenador_pokemon*);
void add_to_pokemon_to_catch(t_pokemon_received* pokemon);

#endif /* TEAM_H_ */
