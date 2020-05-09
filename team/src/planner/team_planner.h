#ifndef PLANNER_TEAM_PLANNER_H_
#define PLANNER_TEAM_PLANNER_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "../logger/team_logger.h"
#include "../config/team_config.h"
#include "../../../shared-common/common/utils.h"

typedef enum
{
	NEW, READY, BLOCK, EXEC, EXIT
} e_state;

typedef struct
{
	int id;
	char* name;
} t_pokemon;

typedef struct
{
	int id;
	e_state state;
	t_position* position;
	t_list* pokemons;
	t_list* targets;
} t_entrenador_pokemon;

t_dictionary* team_planner_global_targets;
void team_planner_init();
void team_planner_destroy();

#endif /* PLANNER_TEAM_PLANNER_H_ */
