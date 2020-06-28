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
	int blocked_time;
	int status; // 0 -> Puede correrse, no espera nada. 1 -> espera un mensaje.
} t_entrenador_info_bloqueo;

typedef struct {
	char* name;
	t_position* position;
} t_pokemon;

typedef struct {
	int id;
	e_state state;
	t_position* position;
	t_list* pokemons;
	t_list* targets;
	int wait_time;
	int current_burst_time;
	int total_burst_time;
	int estimated_time;
	t_entrenador_info_bloqueo* blocked_info;
	pthread_t hilo_entrenador;
	pthread_mutex_t sem_move_trainers;
	t_list* list_id_catch;
	t_pokemon* pokemon_a_atrapar; 
	bool deadlock;
} t_entrenador_pokemon;

typedef enum {
	UNKNOWN,
	FREE,
	BLOCKED
} t_pokemon_state;

typedef struct {
	char* name;
	t_list* pos;
} t_pokemon_received;


sem_t sem_entrenadores_disponibles; //avisa cuando hay entrenadores en la cola de nuevos
pthread_mutex_t sem_message_on_queue; //avisa al algoritmo de cercania cuando hay mensajes encolados
sem_t sem_pokemons_in_ready_queue; //avisa cuando hay pokemons en ready para planificar
sem_t sem_pokemons_to_get; //para enviar al mensaje get
sem_t sem_deadlock; //para controlar deadlock
sem_t sem_planificador; 

t_list* new_queue;
t_list* ready_queue;
t_list* block_queue;
t_list* exit_queue;
t_list* pokemon_to_catch;
t_list* keys_list;
t_list* target_pokemons;

t_dictionary* team_planner_global_targets;

void team_planner_init();
void team_planner_destroy();
void team_planner_run_planification();
void team_planner_algoritmo_cercania();
void move_trainers_and_catch_pokemon();
void team_planner_change_block_status_by_trainer(int, t_entrenador_pokemon*);
t_entrenador_pokemon* team_planner_set_algorithm();
t_list* team_planner_create_ready_queue();
void team_planner_change_block_status_by_id_corr(int, uint32_t);
void team_planner_finish_trainner(t_entrenador_pokemon*);
t_entrenador_pokemon* find_trainer_by_id_corr(uint32_t);
void delete_from_bloqued_queue(t_entrenador_pokemon*, int);
float team_planner_calculate_exponential_mean(int, float);
bool team_planner_is_SJF_algorithm();
void new_cpu_cicle();
void add_to_ready_queue(t_entrenador_pokemon*);
void delete_from_new_queue(t_entrenador_pokemon*);
t_entrenador_pokemon* entrenador_que_necesita(char*);
bool block_queue_is_not_empty();
char* ver_a_quien_no_necesita(t_entrenador_pokemon*);
void remove_from_pokemons_list(t_entrenador_pokemon*, char*);
bool trainer_completed_with_success(t_entrenador_pokemon*);
bool all_queues_are_empty_except_block();
void entrenadores_listos();
void team_planner_print_fullfill_target();
void solve_deadlock();
int team_planner_get_least_estimate_index();
t_pokemon* team_planner_pokemon_create(char* nombre);
t_entrenador_pokemon* team_planner_apply_RR();
t_entrenador_pokemon* team_planner_apply_FIFO();
t_entrenador_pokemon* team_planner_apply_SJF();

#endif /* PLANNER_TEAM_PLANNER_H_ */
