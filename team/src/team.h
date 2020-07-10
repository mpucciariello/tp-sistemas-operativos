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
t_list* get_id_corr;
t_list* keys_list;
t_list* lista_auxiliar;

int team_load();
void team_init();
void team_server_init();
void *handle_connection(void *);
void team_exit();
void *receive_msg(int, int);
void send_ack(void*);
void subscribe_to(void *);
void send_message_test();
void team_retry_connect(void*);
bool pokemon_required(char*);
bool pokemon_not_pendant(char*);
void send_get_message();
void move_trainers_and_catch_pokemon(t_entrenador_pokemon*);
int send_message(void*, t_protocol, t_list*);
bool trainer_is_in_deadlock_caught(t_entrenador_pokemon*);
void send_message_catch(t_catch_pokemon*, t_entrenador_pokemon*);
void add_to_pokemon_to_catch(t_pokemon_received*);
void quitar_de_pokemones_pendientes(char*);
bool pokemon_in_pokemon_to_catch(char*);
void quitar_de_real_target(char*);
bool all_finished();
bool pokemon_not_localized(char*);
<<<<<<< HEAD
void atrapar_pokemon(t_entrenador_pokemon*);
bool todavia_quedan_pokemones_restantes(char*);
void remover_totalmente_de_pokemon_to_catch(char*);
bool tengo_en_pokemon_to_catch(char*);
=======
void atrapar_pokemon(t_entrenador_pokemon*, char*);
>>>>>>> d0d3cff9bd2a9de257e967edf62358ad351a3079


#endif /* TEAM_H_ */
