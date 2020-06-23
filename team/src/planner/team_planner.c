#include "team_planner.h"

bool preemptive;
int fifo_index = 0;
char* split_char = "|";
int deadlocks_detected, deadlocks_resolved = 0, context_switch_qty = 0;

void team_planner_run_planification() {
	sem_wait(&sem_planification);
	sem_wait(&sem_pokemons_in_ready_queue);
	
	team_planner_set_algorithm();

	sem_post(&exec_entrenador->sem_trainer);
	context_switch_qty++;
}

void team_planner_algoritmo_cercania() {
	sem_wait(&sem_entrenadores_disponibles);
	sem_wait(&sem_message_on_queue);
	
	t_temporal_pokemon* pokemon;
	t_entrenador_pokemon* entrenador;
	int c = -1;
	int min_steps = 0;

	t_list* entrenadores_disponibles = list_create();
	entrenadores_disponibles = team_planner_create_ready_queue();
	
	for (int i = 0; i < list_size(entrenadores_disponibles); i++) {
		t_entrenador_pokemon* entrenador_aux = list_get(entrenadores_disponibles, i);
		for (int j = 0; j < list_size(pokemon_to_catch); j++) {
			t_pokemon_received* pokemon_con_posiciones_aux = list_get(pokemon_to_catch, j);
			for (int k = 0; k < list_size(pokemon_con_posiciones_aux->pos); k++) {
				t_position* posicion_aux = list_get(pokemon_con_posiciones_aux->pos, k);

				int aux_x = posicion_aux->pos_x - entrenador_aux->position->pos_x;
				int aux_y = posicion_aux->pos_y - entrenador_aux->position->pos_y;

				int closest_sum = fabs(aux_x + aux_y);

				if (c == -1) {
					min_steps = closest_sum;
					c = 0;
				}

				if (closest_sum < min_steps) {
					pokemon = malloc(sizeof(t_temporal_pokemon));
					pokemon->name = string_duplicate(pokemon_con_posiciones_aux->name);
					pokemon->position->pos_x = posicion_aux->pos_x;
					pokemon->position->pos_y = posicion_aux->pos_y;
					entrenador = entrenador_aux;
				}
			}
		}
	}

	team_logger_info("Se libera al entrenador: id %d", entrenador->id); 	
	
	if(team_planner_is_SJF_algorithm()){
		entrenador->estimated_time = team_planner_calculate_exponential_mean(entrenador->current_burst_time, entrenador->estimated_time);
		team_logger_info("Estimación recalculada del entrenador: %f", entrenador->estimated_time);
		entrenador->current_burst_time = 0;
	}

	if (team_config->algoritmo_planificacion == RR) {
		entrenador->current_burst_time = 0;
	}

	entrenador->pokemon_a_atrapar->name = pokemon->name;
	entrenador->pokemon_a_atrapar->position->pos_x = pokemon->position->pos_x;
	entrenador->pokemon_a_atrapar->position->pos_y = pokemon->position->pos_y;

	pthread_mutex_lock(&planner_mutex);
	list_add(ready_queue, entrenador);
	delete_from_bloqued_queue(entrenador);
	pthread_mutex_unlock(&planner_mutex);
		
	sem_post(&sem_algoritmo_cercania);
}

void delete_from_bloqued_queue(t_entrenador_pokemon* entrenador){	
	for(int i = 0; i < list_size(block_queue); i++){
		t_entrenador_pokemon* entrenador_aux = list_get(block_queue, i);
		if(entrenador_aux->id == entrenador->id){
			pthread_mutex_lock(&planner_mutex);
			list_remove(block_queue, i);
			pthread_mutex_unlock(&planner_mutex);
		}
	}
}


t_entrenador_pokemon* team_planner_entrenador_create(int id_entrenador, t_position* posicion, t_list* pokemons, t_list* targets) {
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
	entrenador->total_burst_time = 0;
	entrenador->estimated_time = 0;
	sem_init(&entrenador->sem_trainer, 0, 0);
	entrenador->blocked_info = NULL;
	entrenador->pokemon_a_atrapar = NULL;
//	pthread_create(&entrenador->hilo_entrenador, NULL, (void*) move_trainers, entrenador);
//	pthread_detach(entrenador->hilo_entrenador);

	return entrenador;
}


t_pokemon* team_planner_pokemon_create(int id, int entrenador_id, char* nombre, t_pokemon_state estado) {
	t_pokemon* pokemon = malloc(sizeof(t_pokemon));
	pokemon->id = id;
	pokemon->trainner_id = entrenador_id;
	pokemon->name = nombre;
	pokemon->state = estado;
	return pokemon;
}


t_pokemon* team_planner_pokemon_appeared_create(char* nombre, int x, int y, t_pokemon_state estado) {
	t_pokemon* pokemon = malloc(sizeof(t_pokemon));
	pokemon->name = nombre;
	pokemon->state = estado;
	t_position* pos = malloc(sizeof(t_position));
	pos->pos_x = x;
	pos->pos_y = y;
	pokemon->position = pos;
	return pokemon;
}


char* team_planner_entrenador_string(t_entrenador_pokemon* entrenador) {
	char* entrenador_string = string_new();
	string_append(&entrenador_string, "ID: ");
	string_append(&entrenador_string, string_itoa(entrenador->id));
	string_append(&entrenador_string, " POSICION: (");
	char* posx = string_itoa(entrenador->position->pos_x);
	string_append(&entrenador_string, posx);
	string_append(&entrenador_string, ", ");
	char* posy = string_itoa(entrenador->position->pos_y);
	string_append(&entrenador_string, posy);
	string_append(&entrenador_string, ") POKEMONS: ");
	for (int i = 0; i < list_size(entrenador->pokemons); i++) {
		t_pokemon* pokemon = list_get(entrenador->pokemons, i);
		string_append(&entrenador_string, pokemon->name);
		if (list_get(entrenador->pokemons, i + 1) != NULL) {
			string_append(&entrenador_string, ", ");
		}
	}
	string_append(&entrenador_string, " OBJETIVOS: ");
	for (int i = 0; i < list_size(entrenador->targets); i++) {
		t_pokemon* pokemon = list_get(entrenador->targets, i);
		string_append(&entrenador_string, pokemon->name);
		if (list_get(entrenador->targets, i + 1) != NULL) {
			string_append(&entrenador_string, ", ");
		}
	}
	return entrenador_string;
}


t_position* team_planner_extract_position(char* pos_spl) {
	char** splitted = string_split(pos_spl, split_char);
	t_position* posicion = malloc(sizeof(t_position));
	posicion->pos_x = atoi(utils_get_parameter_i(splitted, 0));
	posicion->pos_y = atoi(utils_get_parameter_i(splitted, 1));

	return posicion;
}


t_list* team_planner_extract_pokemons(char* pokes_spl, int trainner_id, t_pokemon_state state) {
	t_list* pokemons = list_create();
	char** splitted = string_split(pokes_spl, split_char);
	int pokes_spl_size = utils_get_array_size(splitted);
	for (int j = 0; j < pokes_spl_size; j++) {
		char* nombre = string_duplicate(utils_get_parameter_i(splitted, j));
		t_pokemon* pokemon = team_planner_pokemon_create(j, trainner_id, nombre, state);
		list_add(pokemons, pokemon);
	}
	return pokemons;
}


void planner_init_global_targets(t_list* objetivos) {
	for (int i = 0; i < list_size(objetivos); i++) {
		t_pokemon* pokemon = list_get(objetivos, i);
		char* pokemon_name = string_duplicate(pokemon->name);
		int cantidad_pokemon = (int) dictionary_get(team_planner_global_targets, pokemon_name);
		if (cantidad_pokemon == 0) {
			dictionary_put(team_planner_global_targets, pokemon_name, (void *) 1);
			list_add(keys_list, pokemon_name);
		} else {
			cantidad_pokemon++;
			dictionary_put(team_planner_global_targets, pokemon_name, (void *) cantidad_pokemon);
		}
	}
}


char* planner_print_global_targets() {
	char* global_targets_string = string_new();
	string_append(&global_targets_string, "POKEMON		|CANTIDAD		\n");
	for (int i = 0; i < list_size(keys_list); i++) {
		char* nombre_pokemon = list_get(keys_list, i);
		int j = 10 - strlen(nombre_pokemon);
		int cantidad_pokemon = (int) dictionary_get(team_planner_global_targets, nombre_pokemon);
		char* target_string = string_new();
		for (int k = 0; k < j; k++) {
			strcat(nombre_pokemon, " ");
		}
		string_append(&target_string, nombre_pokemon);
		string_append(&target_string, "		| ");
		string_append(&target_string, string_itoa(cantidad_pokemon));
		string_append(&target_string, "		\n");
		string_append(&global_targets_string, target_string);
	}
	return global_targets_string;
}


void team_planner_add_new_trainner(t_entrenador_pokemon* entrenador) {
	list_add(new_queue, entrenador);
	sem_post(&sem_entrenadores_disponibles);
}


void team_planner_free_pokemons(t_entrenador_pokemon* entrenador) {
	int i;
	for (i = 0; i < list_size(target_pokemons); i++) {
		t_pokemon* p = list_get(target_pokemons, i);

		if (p->blocking_trainner->id == entrenador->id) {
			p->blocking_trainner = NULL;
			p->state = FREE;
		}
	}
}


void team_planner_finish_trainner(t_entrenador_pokemon* entrenador) {
	entrenador->state = EXIT;
	entrenador->blocked_info->blocked_time = 0;
	entrenador->blocked_info->status = 0;
	entrenador->pokemon_a_atrapar = NULL;
	pthread_mutex_lock(&planner_mutex);
	list_add(exit_queue, entrenador);
	pthread_mutex_unlock(&planner_mutex);
	team_planner_free_pokemons(entrenador);
}


void team_planner_change_block_status_by_id_corr(int status, uint32_t id_corr) {
	t_entrenador_info_bloqueo* info_bloqueo = malloc(sizeof(t_entrenador_info_bloqueo)); 
	info_bloqueo->blocked_time = 0;
	info_bloqueo->status = status;
	
	
	t_entrenador_pokemon* entrenador = find_trainer_by_id_corr(id_corr);
	entrenador->state = BLOCK;
	entrenador->pokemon_a_atrapar = NULL;

	if (status == 0) {		
		entrenador->blocked_info = info_bloqueo;
		sem_post(&sem_entrenadores_disponibles);
	}

	if(status == 1 || status == 2) {
		entrenador->blocked_info = info_bloqueo;
	}

	add_to_block_queue_if_not_there(entrenador);
}

void team_planner_change_block_status_by_trainer(int status, t_entrenador_pokemon* entrenador) {
	t_entrenador_info_bloqueo* info_bloqueo = malloc(sizeof(t_entrenador_info_bloqueo)); 
	info_bloqueo->blocked_time = 0;
	info_bloqueo->status = status;
	entrenador->state = BLOCK;
	entrenador->pokemon_a_atrapar = NULL;
	
	entrenador->blocked_info = info_bloqueo;

	if (status == 0) {		
		sem_post(&sem_entrenadores_disponibles);
	}
	add_to_block_queue_if_not_there(entrenador);
}


void add_to_block_queue_if_not_there(t_entrenador_pokemon* entrenador) {
	for (int i = 0; i < list_size(block_queue); i++) {
		t_entrenador_pokemon* entrenador_aux = list_get(block_queue, i);
		if (entrenador_aux->id == entrenador->id){
			continue;
		} else {
			list_add(block_queue, entrenador);
		}
	}
}

t_entrenador_pokemon* find_trainer_by_id_corr(uint32_t id) { 
	int _id_match(t_entrenador_pokemon *entrenador) {
		for (int i = 0; i < list_size(entrenador->list_id_catch); i++) {
			uint32_t id_corr = (uint32_t) list_get(entrenador->list_id_catch, i);
			if (id_corr == id) {
				return 1;
			}
		}
		return 0;
	}
	return list_find(block_queue, (void*) _id_match);
}


bool team_planner_has_config_trainners(int i) {
	return team_config->posiciones_entrenadores[i] != NULL &&
		   team_config->pokemon_entrenadores[i] != NULL &&
		   team_config->objetivos_entrenadores[i] != NULL;
}


void planner_load_entrenadores() {
	int i = 0;

	keys_list = list_create();
	target_pokemons = list_create();
	team_planner_global_targets = dictionary_create();
	while (team_planner_has_config_trainners(i)) {
		t_position* posicion = team_planner_extract_position(team_config->posiciones_entrenadores[i]);
		t_list* pokemons = team_planner_extract_pokemons(team_config->pokemon_entrenadores[i], i, BLOCKED); //TODO: cambiar estado de pokemones atrapados y al llegar
		t_list* objetivos = team_planner_extract_pokemons(team_config->objetivos_entrenadores[i], i, FREE);

		t_entrenador_pokemon* entrenador = team_planner_entrenador_create(i, posicion, pokemons, objetivos);
		team_logger_info("Se creo el entrenador: %s", team_planner_entrenador_string(entrenador));
		team_planner_add_new_trainner(entrenador);
		list_add_all(target_pokemons, objetivos);
		planner_init_global_targets(objetivos);
		i++;
	}
	team_logger_info("Hay %d entrenadores en la cola de NEW", list_size(new_queue));
	int tamanio_objetivos = dictionary_size(team_planner_global_targets);
	char* objetivos_to_string = planner_print_global_targets();
	team_logger_info("Hay %d objetivos globlales: \n%s", tamanio_objetivos, objetivos_to_string);
}


void planner_init_quees() {
	exec_entrenador = malloc(sizeof(t_entrenador_pokemon));
	new_queue = list_create();
	ready_queue = list_create();
	block_queue = list_create();
	exit_queue = list_create();
}

int team_planner_get_least_estimate_index() {
	int least_index = 0;
	float lower_estimate = 100000.0;
	for (int i = 0; i < list_size(ready_queue); i++) {
		t_entrenador_pokemon* entrenador = list_get(ready_queue, i);
		if (entrenador == NULL)
			continue;
		if (entrenador->estimated_time < lower_estimate) {
			lower_estimate = entrenador->estimated_time;
			least_index = i;
		}
	}

	return least_index;
}


void new_cpu_cicle() {
	int i = 0;
	pthread_mutex_lock(&planner_mutex);

	for (i = 0; i < list_size(ready_queue); i++) {
		t_entrenador_pokemon* trainner = list_get(ready_queue, i);
		trainner->wait_time++;
	}

	for (i = 0; i < list_size(block_queue); i++) {
		t_entrenador_pokemon* trainner = list_get(block_queue, i);
		trainner->blocked_info->blocked_time++;
	}

	pthread_mutex_unlock(&planner_mutex);
}

float team_planner_calculate_exponential_mean(int burst_time, float tn) {
	float alpha = 0.5f;
	//tn+1 = α*tn + (1 - α)*tn
	float next_tn = alpha * (float) burst_time + (1.0 - alpha) * tn;
	return next_tn;
}


void team_planner_exec_trainer() {
	exec_entrenador->wait_time = 0;
	exec_entrenador->current_burst_time = 0;
	exec_entrenador->state = EXEC;
	sem_post(&exec_entrenador->sem_trainer);
}


bool team_planner_is_SJF_algorithm() {
	return team_config->algoritmo_planificacion == SJF_CD || team_config->algoritmo_planificacion == SJF_SD;
}


t_pokemon_state team_planner_get_pokemon_state(int id_entrenador, char* pokemon_name) {
	for (int i = 0; i < list_size(target_pokemons); i++) {
		t_pokemon* pokemon = list_get(target_pokemons, i);
		if (pokemon->trainner_id == id_entrenador && string_equals_ignore_case(pokemon->name, pokemon_name)) {
			return pokemon->state;
		}
	}
	return UNKNOWN;
}


t_list* filter_block_list() {
	bool _is_available(t_entrenador_pokemon* trainner) {
		return trainner->blocked_info->status == 0;
	}
	t_list* blocked_but_to_exec = list_filter(block_queue, (void*) _is_available);
	return blocked_but_to_exec;
}


t_list* team_planner_create_ready_queue() {	
	bool _is_available(t_entrenador_pokemon* trainner) {
		return trainner->blocked_info->status == 0;
	}
	t_list* bloquados_en_cero = list_filter(block_queue, (void*) _is_available);
	
	t_list* listo_para_planificar = list_create();
	list_add_all(listo_para_planificar, bloquados_en_cero);
	list_add_all(listo_para_planificar, new_queue);

	return listo_para_planificar;
}


//SJF
void team_planner_apply_SJF(bool is_preemptive) {
	preemptive = is_preemptive;
	pthread_mutex_lock(&planner_mutex);

	int least_estimate_index = team_planner_get_least_estimate_index();

	if (!preemptive) {
		// Hay desalojo
		if (exec_entrenador == NULL && exec_entrenador->pokemon_a_atrapar == NULL) {
			exec_entrenador = list_get(ready_queue, least_estimate_index);
			list_remove(ready_queue, least_estimate_index);
		} else {
			t_entrenador_pokemon* entrenador_menor_estimacion = list_get(ready_queue, least_estimate_index);

			if (entrenador_menor_estimacion->id != exec_entrenador->id) {
				if (entrenador_menor_estimacion->estimated_time < exec_entrenador->estimated_time) {
					list_add(ready_queue, exec_entrenador);
					exec_entrenador = entrenador_menor_estimacion;
					list_remove(ready_queue, least_estimate_index);
				}
				team_planner_exec_trainer();
			}
		}
	} else {
		if (exec_entrenador == NULL) {
			exec_entrenador = list_get(ready_queue, least_estimate_index);
			list_remove(ready_queue, least_estimate_index);			
		}
		team_planner_exec_trainer();
	}
	
	pthread_mutex_unlock(&planner_mutex);
}

//FIFO
void team_planner_apply_FIFO() {
	pthread_mutex_lock(&planner_mutex);

	if (exec_entrenador == NULL && exec_entrenador->pokemon_a_atrapar == NULL) {
		int next_out_index = fifo_index;
		if (next_out_index <= list_size(ready_queue)) { 
			exec_entrenador = list_get(ready_queue, next_out_index);
			list_remove(ready_queue, next_out_index);
			fifo_index++;
		}		
	}
	team_planner_exec_trainer();

	pthread_mutex_unlock(&planner_mutex);
}


//RR
void team_planner_apply_RR() {
	pthread_mutex_lock(&planner_mutex);

	if (exec_entrenador == NULL) {
		int next_out_index = fifo_index;
		if (next_out_index < list_size(ready_queue)) {
			exec_entrenador = list_get(ready_queue, next_out_index);
			list_remove(ready_queue, next_out_index);
			fifo_index++;

		}
		team_planner_exec_trainer();
	}

	pthread_mutex_unlock(&planner_mutex);
}


void team_planner_set_algorithm() {
	switch (team_config->algoritmo_planificacion) {
		case FIFO:
			team_planner_apply_FIFO();
			break;
		case RR:
			team_planner_apply_RR();
			break;
		case SJF_CD:
			team_planner_apply_SJF(false);
			break;
		case SJF_SD:
			team_planner_apply_SJF(true);
			break;
		default:
			break;
	}
}

//t_pokemon* team_planner_get_pokemon_by_trainner(t_entrenador_pokemon* entrenador) {
//	char* pokemon_unneeded = entrenador->blocked_info->pokemon_unneeded;
//	for (int i = 0; i < list_size(target_pokemons); i++) {
//		t_pokemon* p = list_get(target_pokemons, i);
//		if (entrenador->id == p->trainner_id && string_equals_ignore_case(pokemon_unneeded, p->name)) {
//			return p;
//		}
//	}
//	return NULL;
//}

/*void team_planner_solve_deadlock(t_entrenador_pokemon* bloqueado, t_entrenador_pokemon* bloqueante) {
	team_logger_info("Se procederá a resolver deadlock!");
	deadlocks_resolved++;

	team_logger_info("El deadlock ha sido resuelto!");
}


//va a poder filtrar por info = 2.
t_list* team_planner_get_id_entrenadores_deadlock(t_entrenador_pokemon* entrenador) {
	t_list* lista = list_create();

	t_pokemon* pokemon_necesita_entrenador_bloqueante = NULL;
	t_pokemon* pokemon_necesita_entrenador = team_planner_get_pokemon_by_trainner(entrenador);

	if (pokemon_necesita_entrenador->state == FREE)
		return NULL;
	// Este es el entrenador que esta bloqueando a mi entrenador original
	t_entrenador_pokemon* entrenador_bloqueante = pokemon_necesita_entrenador->blocking_trainner;
	// Si no hay entrenador bloqueante => retorno
	if (entrenador_bloqueante == NULL)
		return NULL;
	list_add(lista, &entrenador_bloqueante->id);

	while (true) {
		if (entrenador_bloqueante->state == BLOCK) {
			// Este es el pokemon que necesita el entrenador bloqueante para continuar
			pokemon_necesita_entrenador_bloqueante = team_planner_get_pokemon_by_trainner(entrenador_bloqueante);
			// Si el pokemon que necesita el entrenador bloqueante esta bloqueado por el entrenador original => hay deadlock
			if (pokemon_necesita_entrenador_bloqueante->blocking_trainner->id == entrenador->id) {
				list_add(lista, &entrenador->id);
				return lista;
			}
			if (entrenador_bloqueante->id == pokemon_necesita_entrenador_bloqueante->blocking_trainner->id)
				return lista;
			// No estaba bloqueado por el original => guardo la info
			list_add(lista, &pokemon_necesita_entrenador_bloqueante->blocking_trainner->id);
			// Evaluo al entrenador que bloquea a mi entrenador bloqueante
			entrenador_bloqueante = pokemon_necesita_entrenador_bloqueante->blocking_trainner;
		} else {
			// El entrenador bloqueante no esta bloqueado
			return NULL;
		}
	}
}

void team_planner_check_deadlocks() {
	t_list* ids_bloqueados = list_create();

	int _get_id(t_entrenador_pokemon* entrenador) {
		return entrenador->id;
	}
	ids_bloqueados = list_map(block_queue, (void*) _get_id);

	int i;
	for (i = 0; i < list_size(block_queue); i++) {
		t_entrenador_pokemon* trainner = list_get(block_queue, i);

		bool _es_el_mismo(int id) {
			return trainner->id == id;
		}
		if (!list_any_satisfy(ids_bloqueados, (void*)_es_el_mismo))
			continue;

		t_list* id_entrenadores_deadlock = team_planner_get_id_entrenadores_deadlock(trainner);

		if (id_entrenadores_deadlock != NULL) {
			if (!list_is_empty(id_entrenadores_deadlock)) {
				deadlocks_detected++;
				team_logger_info("Se ha detectado deadlock entre:");
				team_logger_info("Entrenador %d", trainner->id);
				int j;
				for (j = 0; j < list_size(id_entrenadores_deadlock) ; j++) {
					int* aux = list_get(id_entrenadores_deadlock, j);
					int k;
					for (k = 0; k < list_size(ids_bloqueados); k++) {
						int id_aux = (int) list_get(ids_bloqueados, k);
						if (id_aux == *aux)
							list_remove(ids_bloqueados, k);
					}
					team_logger_info("Bloqueado por %d", *aux);

					int _is_the_one(t_entrenador_pokemon* entrenador) {
						return entrenador->id = *aux;
					}
					t_entrenador_pokemon* trainner_bloqueado = list_find(block_queue, (void*) _is_the_one);

					team_planner_solve_deadlock(trainner_bloqueado, trainner);
				}
				list_clean(id_entrenadores_deadlock);
			} else {
				team_logger_warn("No se ha detectado deadlock");
			}
		}
	}
	list_destroy(ids_bloqueados);
}*/

t_list* team_planner_get_trainners() {
	t_list* trainners = list_create();
	list_add(trainners, exec_entrenador);
	list_add_all(trainners, new_queue);
	list_add_all(trainners, ready_queue);
	list_add_all(trainners, block_queue);
	list_add_all(trainners, exit_queue);
  return trainners;
}

void team_planner_print_fullfill_target() {
	pthread_mutex_lock(&planner_mutex);
	t_list* trainners = team_planner_get_trainners();

	int add_burst_time(int accum, t_entrenador_pokemon* trainner) {
      return accum + trainner->current_burst_time;
	}
	int total_cpu = (int) list_fold(trainners, 0, (void*) add_burst_time);
	team_logger_info("Cantidad de ciclos de CPU totales: %d", total_cpu);
	team_logger_info("Cantidad de cambios de contexto realizados: %d", context_switch_qty);
	team_logger_info("Cantidad de ciclos de CPU realizados por entrenador:");

	void _list_burst_trainner(t_entrenador_pokemon *trainner) {
		team_logger_info("Entrenador %d -> Ciclos de CPU realizados: %d", trainner->id, trainner->total_burst_time);
	}
	list_iterate(trainners, (void*) _list_burst_trainner);

	list_destroy(trainners);
	team_logger_info("Deadlocks producidos: %d  y resueltos: %d", deadlocks_detected, deadlocks_resolved);
	pthread_mutex_unlock(&planner_mutex);
}

void team_planner_init() {
	team_logger_info("Planificador de TEAM iniciando estructuras!");
	planner_init_quees();
	planner_load_entrenadores();
	sem_init(&sem_planification, 0, 1); 
	sem_init(&sem_algoritmo_cercania, 0, 0);
	sem_init(&sem_pokemons_in_ready_queue, 0, 0);
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
	dictionary_destroy_and_destroy_elements(global_targets, (void*)planner_destroy_pokemons);
}

void planner_destroy_quees() {
	planner_destroy_entrenador(exec_entrenador);
	list_destroy_and_destroy_elements(new_queue, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(ready_queue, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(block_queue, (void*)planner_destroy_entrenador);
	list_destroy_and_destroy_elements(exit_queue, (void*)planner_destroy_entrenador);	
	list_destroy_and_destroy_elements(pokemon_to_catch, (void*)planner_destroy_entrenador);
	list_destroy(keys_list);
	list_destroy(target_pokemons);
}

void team_planner_destroy() {
	sem_destroy(&sem_entrenadores_disponibles);
	sem_destroy(&sem_message_on_queue);
	sem_destroy(&sem_planification);
	sem_destroy(&sem_pokemons_in_ready_queue);
	sem_destroy(&sem_algoritmo_cercania);
	planner_destroy_quees();
	planner_destroy_global_targets(team_planner_global_targets);
}
