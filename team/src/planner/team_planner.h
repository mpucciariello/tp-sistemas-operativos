#ifndef PLANNER_TEAM_PLANNER_H_
#define PLANNER_TEAM_PLANNER_H_
#include <stdio.h>
#include <stdlib.h>
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
	char* pokemon_needed;
	int blocked_time;
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

sem_t sem_entrenadores;
t_dictionary* team_planner_global_targets;
t_list* pokemon_to_catch;

void team_planner_init();
void team_planner_destroy();

#endif /* PLANNER_TEAM_PLANNER_H_ */
