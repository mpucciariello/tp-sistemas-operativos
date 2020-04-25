#include "team_planner.h"

t_entrenador_pokemon* exec_entrenador;
t_list* new_queque;
t_list* ready_queque;
t_list* block_queque;
t_list* exit_queque;

t_entrenador_pokemon* planner_entrenador_create(int id_entrenador, t_position* posicion, t_list* pokemons, t_list* targets)
{
	t_entrenador_pokemon* entrenador = malloc(sizeof(t_entrenador_pokemon));
	entrenador->id = id_entrenador;
	entrenador->state = NEW;
	entrenador->position = malloc(sizeof(t_position));
	entrenador->position = posicion;
	entrenador->pokemons = list_create();
	entrenador->pokemons = pokemons;
	entrenador->targets = list_create();
	entrenador->targets = targets;

	return entrenador;
}

char* planner_entrenador_string(t_entrenador_pokemon* entrenador) {
	char* entrenador_string = string_new();
	string_append(&entrenador_string, "Id: ");
	string_append(&entrenador_string, string_itoa(entrenador->id));
	string_append(&entrenador_string, " Pokemons: ");
	for(int i = 0; i < list_size(entrenador->pokemons); i++) {
		t_pokemon* pokemon = list_get(entrenador->pokemons, i);
		string_append(&entrenador_string, pokemon->name);
		if(list_get(entrenador->pokemons, i + 1) != NULL) {
			string_append(&entrenador_string, ", ");
		}
	}
	string_append(&entrenador_string, " Objetivos: ");
	for(int i = 0; i < list_size(entrenador->targets); i++) {
		t_pokemon* pokemon = list_get(entrenador->targets, i);
		string_append(&entrenador_string, pokemon->name);
		if(list_get(entrenador->targets, i + 1) != NULL) {
			string_append(&entrenador_string, ", ");
		}
	}
	return entrenador_string;
}

t_position* planner_extract_position(char** pos_spl) {
	t_position* posicion = malloc(sizeof(t_position));
	posicion->pos_x = atoi(utils_get_parameter_i(pos_spl, 0));
	posicion->pos_y = atoi(utils_get_parameter_i(pos_spl, 1));

	return posicion;
}

t_list* planner_extract_pokemons(char** pokes_spl) {
	t_list* pokemons = list_create();
	int pokes_spl_size = utils_get_array_size(pokes_spl);
	for(int j = 0; j < pokes_spl_size; j++) {
		t_pokemon* pokemon = malloc(sizeof(t_pokemon));
		pokemon->id = j;
		pokemon->name = string_duplicate(utils_get_parameter_i(pokes_spl, j));
		list_add(pokemons, pokemon);
	}
	return pokemons;
}

void planner_load_entrenadores() {
	int i = 0;
	char* split_char = "|";
	while(team_config->posiciones_entrenadores[i] != NULL && team_config->pokemon_entrenadores[i] != NULL &&
		  team_config->objetivos_entrenadores[i] != NULL) {

		char** pos_spl = string_split(team_config->posiciones_entrenadores[i], split_char);
		t_position* posicion = planner_extract_position(pos_spl);

		char** pokes_spl = string_split(team_config->pokemon_entrenadores[i], split_char);
		t_list* pokemons = planner_extract_pokemons(pokes_spl);

		char** target_spl = string_split(team_config->objetivos_entrenadores[i], split_char);
		t_list* objetivos = planner_extract_pokemons(target_spl);

		t_entrenador_pokemon* entrenador = planner_entrenador_create(i, posicion, pokemons, objetivos);
		list_add(new_queque, entrenador);

		team_logger_info("Se creo el entrenador: %s", planner_entrenador_string(entrenador));
		i++;
	}
	team_logger_info("Hay %d entrenadores en la cola de NEW", list_size(new_queque));
}

void planner_init_quees()
{
	new_queque = list_create();
	ready_queque = list_create();
	block_queque = list_create();
	exit_queque = list_create();
}

void team_planner_init()
{
	team_logger_info("Planificador de TEAM iniciando estructuras!");
	planner_init_quees();
	planner_load_entrenadores();
}

void planner_destroy_entrenador(t_entrenador_pokemon* entrenador)
{
	free(entrenador->position);
	list_destroy(entrenador->pokemons);
	free(entrenador->pokemons);
	list_destroy(entrenador->targets);
	free(entrenador->targets);
	free(entrenador);
}

void planner_destroy_quees()
{
	list_destroy_and_destroy_elements(new_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(ready_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(block_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(exit_queque, (void*)planner_destroy_entrenador);
}
