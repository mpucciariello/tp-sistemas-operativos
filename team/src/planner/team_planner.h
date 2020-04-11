#ifndef PLANNER_TEAM_PLANNER_H_
#define PLANNER_TEAM_PLANNER_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include "../logger/team_logger.h"
#include "../config/team_config.h"
#include "../../../shared-common/common/utils.h"

typedef struct
{
	int x;
	int y;
} t_position;

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
	t_pokemon** pokemons;
	t_pokemon** targets;
} t_entrenador_pokemon;

void team_planner_init();

#endif /* PLANNER_TEAM_PLANNER_H_ */
