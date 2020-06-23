#ifndef PLANNER_TEAM_PLANNER_H_
#define PLANNER_TEAM_PLANNER_H_
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include "../logger/team_logger.h"
#include "../config/team_config.h"
#include "../../../shared-common/common/utils.h"

typedef enum {
	NEW, READY, BLOCK, EXEC, EXIT
} e_state;

typedef struct {
	char* pokemon_unneeded;
	int blocked_time;
	int status; // 0 -> Puede correrse, no espera nada. 1 -> espera un mensaje. 2 -> Deadlock
} t_entrenador_info_bloqueo;

typedef struct {
	int id;
	e_state state;
	t_position* position;
	t_list* pokemons;
	t_list* targets;
	int wait_time;
	int current_burst_time;
	int estimated_time;
	t_entrenador_info_bloqueo* blocked_info;
	sem_t sem_trainer;
	pthread_t hilo_entrenador;
	t_list* list_id_catch;
} t_entrenador_pokemon;

typedef enum {
	UNKNOWN,
	FREE,
	BLOCKED
} t_pokemon_state;

typedef struct {
	int id;
	int trainner_id;
	char* name;
	t_pokemon_state state;
	t_entrenador_pokemon* blocking_trainner;
	t_position* position;
} t_pokemon;

typedef struct {
	char* name;
	t_list* pos;
} t_pokemon_received;

typedef struct {
	char* name;
	t_position* position;
} t_temporal_pokemon;

sem_t sem_entrenadores_disponibles; //avisa cuando hay entrenadores en la cola de nuevos
sem_t sem_message_on_queue; //avisa al algoritmo de cercania cuando hay mensajes encolados
sem_t sem_planification; //controla que el pokemon permita al planificador seguir
sem_t sem_pokemons_in_ready_queue; //avisa cuando hay pokemons en ready para planificar
sem_t sem_algoritmo_cercania; //para añadir a la cola de ready

pthread_mutex_t planner_mutex;
t_temporal_pokemon* pokemon_temporal;
t_entrenador_pokemon* exec_entrenador;

t_list* new_queue;
t_list* ready_queue;
t_list* block_queue;
t_list* exit_queue;
t_list* pokemon_to_catch;
t_list* pokemons_ready;
t_list* keys_list;
t_list* target_pokemons;

t_dictionary* team_planner_global_targets;

void team_planner_init();
void team_planner_destroy();
void team_planner_run_planification();
void team_planner_algoritmo_cercania();
void move_trainers();
void team_planner_change_block_status_by_trainer(int, t_entrenador_pokemon*, char*);
void team_planner_set_algorithm();
t_list* team_planner_create_ready_queue();
void team_planner_change_block_status_by_id_corr(int, uint32_t, char*);
void team_planner_finish_trainner(t_entrenador_pokemon*);
t_entrenador_pokemon* find_trainer_by_id_corr(uint32_t);
void delete_from_bloqued_queue(t_entrenador_pokemon*);
float team_planner_calculate_exponential_mean(int, float);
bool team_planner_is_SJF_algorithm();
void new_cpu_cicle();
void add_to_block_queue_if_not_there(t_entrenador_pokemon*);

#endif /* PLANNER_TEAM_PLANNER_H_ */
