#include "team_planner.h"

t_entrenador_pokemon* exec_entrenador;
t_list* new_queque;
t_list* ready_queque;
t_list* block_queque;
t_list* exit_queque;

t_list* keys_list;

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
	string_append(&entrenador_string, " Posicion: (");
	char* posx = string_itoa(entrenador->position->pos_x);
	string_append(&entrenador_string, posx);
	string_append(&entrenador_string, ", ");
	char* posy = string_itoa(entrenador->position->pos_y);
	string_append(&entrenador_string, posy);
	string_append(&entrenador_string, ") Pokemons: ");
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

t_position* planner_extract_position(char* pos_spl, char* split_char) {
	char** splitted = string_split(pos_spl, split_char);
	t_position* posicion = malloc(sizeof(t_position));
	posicion->pos_x = atoi(utils_get_parameter_i(splitted, 0));
	posicion->pos_y = atoi(utils_get_parameter_i(splitted, 1));

	return posicion;
}

t_list* planner_extract_pokemons(char* pokes_spl, char* split_char) {
	t_list* pokemons = list_create();
	char** splitted = string_split(pokes_spl, split_char);
	int pokes_spl_size = utils_get_array_size(splitted);
	for(int j = 0; j < pokes_spl_size; j++) {
		t_pokemon* pokemon = malloc(sizeof(t_pokemon));
		pokemon->id = j;
		pokemon->name = string_duplicate(utils_get_parameter_i(splitted, j));
		list_add(pokemons, pokemon);
	}
	return pokemons;
}

void planner_init_global_targets(t_list* objetivos) {
	for(int i = 0; i < list_size(objetivos); i++) {
		t_pokemon* pokemon = list_get(objetivos, i);
		char* pokemon_name = string_duplicate(pokemon->name);
		int cantidad_pokemon = (int) dictionary_get(team_planner_global_targets, pokemon_name);
		if(cantidad_pokemon == 0) {
			dictionary_put(team_planner_global_targets, pokemon_name, (int) 1);
			list_add(keys_list, pokemon_name);
		} else {
			cantidad_pokemon++;
			dictionary_put(team_planner_global_targets, pokemon_name, (int) cantidad_pokemon);
		}
	}
}

char* planner_print_global_targets() {
	char* global_targets_string = string_new();
	string_append(&global_targets_string, "Pokemon		|Cantidad		\n");
	for(int i = 0; i < list_size(keys_list); i++) {
		char* nombre_pokemon = list_get(keys_list, i);
		int cantidad_pokemon = (int) dictionary_get(team_planner_global_targets, nombre_pokemon);
		char* target_string = string_new();
		string_append(&target_string, nombre_pokemon);
		string_append(&target_string, "		|");
		string_append(&target_string, string_itoa(cantidad_pokemon));
		string_append(&target_string, "		\n");

		string_append(&global_targets_string, target_string);
	}
	return global_targets_string;
}

void planner_load_entrenadores() {
	int i = 0;
	char* split_char = "|";
	keys_list = list_create();
	team_planner_global_targets = dictionary_create();
	while(team_config->posiciones_entrenadores[i] != NULL && team_config->pokemon_entrenadores[i] != NULL &&
					team_config->objetivos_entrenadores[i] != NULL) {
		t_position* posicion = planner_extract_position(team_config->posiciones_entrenadores[i], split_char);
		t_list* pokemons = planner_extract_pokemons(team_config->pokemon_entrenadores[i], split_char);
		t_list* objetivos = planner_extract_pokemons(team_config->objetivos_entrenadores[i], split_char);

		t_entrenador_pokemon* entrenador = planner_entrenador_create(i, posicion, pokemons, objetivos);
		list_add(new_queque, entrenador);

		planner_init_global_targets(objetivos);
		team_logger_info("Se creo el entrenador: %s", planner_entrenador_string(entrenador));
		i++;
	}
	team_logger_info("Hay %d entrenadores en la cola de NEW", list_size(new_queque));
	team_logger_info("Hay %d objetivos globlales: \n%s", dictionary_size(team_planner_global_targets), planner_print_global_targets());
}

void planner_init_quees()
{
	exec_entrenador = malloc(sizeof(t_entrenador_pokemon));
	new_queque = list_create();
	ready_queque = list_create();
	block_queque = list_create();
	exit_queque = list_create();
}

float team_planner_calculate_exponential_mean(int burst_time)
{
	float tn = 0.0f;
	float alpha = 0.5f;
	//tn+1 = α*tn + (1 - α)*tn
	float next_tn = alpha * (float) burst_time + (1.0 - alpha) * tn;
	return next_tn;
}

void team_planner_set_algorithm() {
	switch(team_config->algoritmo_planificacion) {
		case FIFO:
			break;
		case RR: {
			int quantum = team_config->quantum;
			team_logger_info("Quantum: %d", quantum);
		}
			break;
		case SJF_CD:
			break;
		case SJF_SD: {
			float initial_estimate = (float) team_config->estimacion_inicial;
			team_logger_info("Estimacion inicial: %f", initial_estimate);
		}
			break;
		default:
			break;
	}
}

void team_planner_init()
{
	team_logger_info("Planificador de TEAM iniciando estructuras!");
	planner_init_quees();
	planner_load_entrenadores();
}

void planner_destroy_pokemons(t_pokemon* pokemon)
{
	free(pokemon->name);
	free(pokemon);
}

void planner_destroy_entrenador(t_entrenador_pokemon* entrenador)
{
	free(entrenador->position);
	list_destroy_and_destroy_elements(entrenador->pokemons, (void*)planner_destroy_pokemons);
	free(entrenador->pokemons);
	list_destroy_and_destroy_elements(entrenador->targets, (void*)planner_destroy_pokemons);
	free(entrenador->targets);
	free(entrenador);
}

void planner_destroy_global_targets(t_dictionary* global_targets) {
	dictionary_destroy_and_destroy_elements(global_targets, planner_destroy_pokemons);
}

void planner_destroy_quees()
{
	planner_destroy_entrenador(exec_entrenador);
	list_destroy_and_destroy_elements(new_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(ready_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(block_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(exit_queque, (void*)planner_destroy_entrenador);
	list_destroy(keys_list);
}

void team_planner_destroy() {
	planner_destroy_quees();
	planner_destroy_global_targets(team_planner_global_targets);
}
