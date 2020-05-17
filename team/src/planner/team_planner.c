#include "team_planner.h"

t_entrenador_pokemon* exec_entrenador;
t_list* new_queque;
t_list* ready_queque;
t_list* block_queque;
t_list* exit_queque;

t_list* keys_list;

bool preemptive;
int fifo_index = 0;

t_entrenador_pokemon* planner_entrenador_create(int id_entrenador, t_position* posicion, t_list* pokemons, t_list* targets) {
	t_entrenador_pokemon* entrenador = malloc(sizeof(t_entrenador_pokemon));
	entrenador->id = id_entrenador;
	entrenador->state = NEW;
	entrenador->position = malloc(sizeof(t_position));
	entrenador->position = posicion;
	entrenador->pokemons = list_create();
	entrenador->pokemons = pokemons;
	entrenador->targets = list_create();
	entrenador->targets = targets;
	entrenador->wait_time = 0;
	entrenador->current_burst_time = 0;
	entrenador->estimated_time = 0;

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
		sem_post(&sem_entrenadores);

		planner_init_global_targets(objetivos);
		team_logger_info("Se creo el entrenador: %s", planner_entrenador_string(entrenador));
		i++;
	}
	team_logger_info("Hay %d entrenadores en la cola de NEW", list_size(new_queque));
	team_logger_info("Hay %d objetivos globlales: \n%s", dictionary_size(team_planner_global_targets), planner_print_global_targets());
}

void planner_init_quees() {
	exec_entrenador = malloc(sizeof(t_entrenador_pokemon));
	new_queque = list_create();
	ready_queque = list_create();
	block_queque = list_create();
	exit_queque = list_create();
}

int team_planner_get_least_estimate_index() {
	int least_index = 0;
	float lower_estimate = 100000.0;
	for (int i = 0; i < list_size(ready_queque); i++) {
		t_entrenador_pokemon* entrenador = list_get(ready_queque, i);
		if (entrenador == NULL)
			continue;
		if (entrenador->estimated_time < lower_estimate) {
			lower_estimate = entrenador->estimated_time;
			least_index = i;
		}
	}

	return least_index;
}

float team_planner_calculate_exponential_mean(int burst_time, float tn) {
	float alpha = 0.5f;
	//tn+1 = α*tn + (1 - α)*tn
	float next_tn = alpha * (float) burst_time + (1.0 - alpha) * tn;
	return next_tn;
}

void team_planner_check_exec_trainer_is_block() {
	if(exec_entrenador != NULL && exec_entrenador->state == BLOCK) {
		team_logger_info("El entrenador actual esta bloqueado, lo mando a la cola de bloqueados");
		exec_entrenador->wait_time = 0;
		// Encolo el entrenador en la lista de bloqueados
		list_add(block_queque, exec_entrenador);
		// Libero el entrenador actual
		exec_entrenador = NULL;
	}
}

void team_planner_admit_new_trainers() {
	int i = 0;
	if (!list_is_empty(new_queque)) {
		//1 - Antes de admitirlos les seteo la estimacion inicial
		for (i = 0; i < list_size(new_queque); i++) {
			t_entrenador_pokemon* trainner = list_get(new_queque, i);
			if (trainner == NULL)
				continue;
			trainner->estimated_time = (float) team_config->estimacion_inicial;
			team_logger_info("¡El entrenador %d ha sido incorporado!", trainner->id);
		}
		//2 - Admitir nuevos entrenadores
		list_add_all(ready_queque, new_queque);
		list_clean(new_queque);
	}
}

bool team_planner_is_SJF_algorithm() {
	return team_config->algoritmo_planificacion == SJF_CD || team_config->algoritmo_planificacion == SJF_SD;
}

void team_planner_check_unlocks() {
	int i;
	bool set_free;
	t_entrenador_pokemon* trainner = NULL;

	for(i = 0; i < list_size(block_queque); i++) {
		set_free = false;
		trainner = list_get(block_queque, i);

		if(trainner == NULL)
			continue;

		//BLOCKED_INFO* infoBloqueo = trainner->informacionDeBloqueo;

		//if(infoBloqueo == NULL) {
			// Si no tiene informacion de bloqueo entonces no esta bloqueado
			//set_free = true;
		//}

		//if(!set_free && estadoRecurso(infoBloqueo->recursoNecesitado) == RECURSO_LIBRE)
			set_free = true;

		if(set_free) {
			team_logger_info("Se libera el entrenador: id %d", trainner->id);
			trainner->estimated_time = team_planner_calculate_exponential_mean(trainner->current_burst_time, esi->estimated_time);
			team_logger_info("Estimacion recalculada: %f", trainner->estimated_time);

			if(team_planner_is_SJF_algorithm()) {
				// Si el algoritmo es SJF ya no me sirve el tiempo de rafaga
				trainner->current_burst_time = 0;
			}
			// Cuando se desbloquea, empieza a contar el tiempo de espera
			trainner->wait_time = 0;
			trainner->state = READY;
			//trainner->informacionDeBloqueo->unidadesDeTiempoBloqueado = 0;
			// Cambio de lista
			list_remove(block_queque, i);
			i--;
			list_add(ready_queque, trainner);
			// Ahora hay un entrenador mas que esta listo para ejecutar
			sem_post(&sem_entrenadores);
		}
	}
}

void team_planner_run_checks() {
	//Si el entrenador actual termino bloqueado lo encolo
	team_planner_check_exec_trainer_is_block();
	//Admitir nuevos entrenadores
	team_planner_admit_new_trainers();
	//Se desbloquea algun entrenador?
	team_planner_check_unlocks();
	//Hay entrenadores listos para ejecutar?
	if(list_is_empty(ready_queque)) {
		team_logger_info("No hay enetrenadores listos para ejecutar");
		pthread_mutex_unlock(&planner_mutex);
		return;
	}
}

void team_planner_apply_SJF(bool is_preemptive) {
	preemptive = is_preemptive;
	pthread_mutex_lock(&planner_mutex);

	team_planner_run_checks();

	int least_estimate_index = team_planner_get_least_estimate_index();

	if(!preemptive) {
		//Hay desalojo
		if(exec_entrenador == NULL) {
			exec_entrenador = list_get(ready_queque, least_estimate_index);
			list_remove(ready_queque, least_estimate_index);
		} else {
			t_entrenador_pokemon* entrenador_menor_estimacion = list_get(ready_queque, least_estimate_index);

			if(entrenador_menor_estimacion->id != exec_entrenador->id) {
				if(entrenador_menor_estimacion->estimated_time < exec_entrenador->estimated_time) {
					list_add(ready_queque, exec_entrenador);
					exec_entrenador = entrenador_menor_estimacion;
					list_remove(ready_queque, least_estimate_index);
				}
			}
		}
	} else {
		if(exec_entrenador == NULL) {
			exec_entrenador = list_get(ready_queque, least_estimate_index);
			list_remove(ready_queque, least_estimate_index);
		}
	}

	exec_entrenador->wait_time = 0;
	pthread_mutex_unlock(&planner_mutex);
}

void team_planner_apply_FIFO() {
	pthread_mutex_lock(&planner_mutex);

	team_planner_run_checks();

	//Si no hay entrenador, planifico el proximo
	if (exec_entrenador == NULL) {
		int next_out_index = fifo_index;
		if(next_out_index < list_size(ready_queque)) {
			exec_entrenador = list_get(ready_queque, next_out_index);
			list_remove(ready_queque, next_out_index);
			fifo_index++;
		}
		exec_entrenador->wait_time = 0;
		exec_entrenador->current_burst_time = 0;
	}

	pthread_mutex_unlock(&planner_mutex);
}

void team_planner_apply_RR() {
	pthread_mutex_lock(&planner_mutex);

	team_planner_run_checks();

	if (exec_entrenador == NULL) {
		int next_out_index = fifo_index;
		if(next_out_index < list_size(ready_queque)) {
			exec_entrenador = list_get(ready_queque, next_out_index);
			list_remove(ready_queque, next_out_index);
			fifo_index++;
		}
		exec_entrenador->wait_time = 0;
		exec_entrenador->current_burst_time = 0;
	}

	pthread_mutex_unlock(&planner_mutex);
}

void team_planner_set_algorithm() {
	switch(team_config->algoritmo_planificacion) {
		case FIFO:
			team_planner_apply_FIFO();
			break;
		case RR:
			team_planner_apply_RR();
			break;
		case SJF_CD:
			team_planner_apply_SJF(true);
			break;
		case SJF_SD:
			team_planner_apply_SJF(false);
			break;
		default:
			break;
	}
}

void team_planner_init() {
	team_logger_info("Planificador de TEAM iniciando estructuras!");
	planner_init_quees();
	planner_load_entrenadores();
	sem_init(&sem_entrenadores, 0, 0);
}

void planner_destroy_pokemons(t_pokemon* pokemon) {
	free(pokemon->name);
	free(pokemon);
}

void planner_destroy_entrenador(t_entrenador_pokemon* entrenador) {
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

void planner_destroy_quees() {
	planner_destroy_entrenador(exec_entrenador);
	list_destroy_and_destroy_elements(new_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(ready_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(block_queque, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(exit_queque, (void*)planner_destroy_entrenador);
	list_destroy(keys_list);
}

void team_planner_destroy() {
	sem_destroy(&sem_entrenadores);
	planner_destroy_quees();
	planner_destroy_global_targets(team_planner_global_targets);
}
