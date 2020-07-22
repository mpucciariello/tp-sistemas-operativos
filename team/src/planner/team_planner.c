#include "team_planner.h"

bool preemptive;
int fifo_index = 0;
char* split_char = "|";
int deadlocks_detected = 0, deadlocks_resolved = 0, context_switch_qty = 0;

void team_planner_run_planification() {
	while (true) {
		sem_wait(&sem_trainers_in_ready_queue);
		sem_wait(&sem_planificador);

		t_entrenador_pokemon* entrenador = team_planner_set_algorithm();

		team_logger_info("El entrenador %d pasará a EXEC!", entrenador->id);
		context_switch_qty++;
		entrenador->previus_estimation = entrenador->estimated_burst;

		pthread_mutex_lock(&cola_exec);
		list_add(exec_queue, entrenador);
		pthread_mutex_unlock(&cola_exec);
		pthread_mutex_unlock(&entrenador->sem_move_trainers);
	}
}

void team_planner_algoritmo_cercania() {
	while (true) {
		sem_wait(&sem_message_on_queue);
		sem_wait(&sem_entrenadores_disponibles);

		t_pokemon* pokemon;
		t_entrenador_pokemon* entrenador = malloc(sizeof(t_entrenador_pokemon));

		int c = -1;
		int min_steps = 0;

		t_list* entrenadores_disponibles = team_planner_create_ready_queue();

		if(!list_is_empty(entrenadores_disponibles)){
			for (int i = 0; i < list_size(entrenadores_disponibles); i++) {
				t_entrenador_pokemon* entrenador_aux = list_get(entrenadores_disponibles, i);
				for (int j = 0; j < list_size(pokemon_to_catch); j++) {
					pthread_mutex_lock(&cola_pokemons_a_atrapar);
					t_pokemon_received* pokemon_con_posiciones_aux = list_get(pokemon_to_catch, j);
					pthread_mutex_unlock(&cola_pokemons_a_atrapar);
					for (int k = 0; k < list_size(pokemon_con_posiciones_aux->pos); k++) {
						t_position* posicion_aux = list_get(pokemon_con_posiciones_aux->pos, k);

						int aux_x = posicion_aux->pos_x - entrenador_aux->position->pos_x;
						int aux_y = posicion_aux->pos_y - entrenador_aux->position->pos_y;

						int closest_sum = fabs(aux_x) + fabs(aux_y);

						if (c == -1) {
							min_steps = closest_sum;
							pokemon = malloc(sizeof(t_pokemon));
							pokemon->name = string_duplicate(pokemon_con_posiciones_aux->name);
							pokemon->position = malloc(sizeof(t_position));
							pokemon->position->pos_x = posicion_aux->pos_x;
							pokemon->position->pos_y = posicion_aux->pos_y;
							entrenador = entrenador_aux;
							c = 0;
						}

						if (closest_sum < min_steps) {
							min_steps = closest_sum;
							pokemon = malloc(sizeof(t_pokemon));
							pokemon->name = string_duplicate(pokemon_con_posiciones_aux->name);
							pokemon->position = malloc(sizeof(t_position));
							pokemon->position->pos_x = posicion_aux->pos_x;
							pokemon->position->pos_y = posicion_aux->pos_y;
							entrenador = entrenador_aux;
						}
					}
				}
			}
		}

		if(entrenador != NULL){
			entrenador->current_burst_time = 0;
			entrenador->pokemon_a_atrapar = pokemon;

			team_planner_add_to_ready_queue(entrenador);
			team_planner_remove_pokemon_from_catch(pokemon);
			list_add(pokemones_pendientes, entrenador->pokemon_a_atrapar->name);
			sem_post(&sem_trainers_in_ready_queue);
		}
	}
}

void team_planner_add_to_ready_queue(t_entrenador_pokemon* entrenador) {
	entrenador->state = READY;
	list_add(ready_queue, entrenador);
	team_planner_delete_from_bloqued_queue(entrenador, 0);
	team_planner_delete_from_new_queue(entrenador);	
}

void team_planner_delete_from_bloqued_queue(t_entrenador_pokemon* entrenador, int cola) {
	for (int i = 0; i < list_size(block_queue); i++) {
		t_entrenador_pokemon* entrenador_aux = list_get(block_queue, i);
		if (entrenador_aux->id == entrenador->id) {
			list_remove(block_queue, i);
			if (cola == 0) {
				if (!entrenador->deadlock) {
					team_logger_info("Se eliminó al entrenador %d de la cola BLOCK porque pasará a READY. Fue seleccionado por el algoritmo de cercanía!", entrenador->id);
				} else {
					team_logger_info("Se eliminó al entrenador %d de la cola BLOCK porque pasará a READY. Fue seleccionado por el algoritmo de detección de deadlocks!", entrenador->id);
				}
			} else {
				team_logger_info("Se eliminó al entrenador %d de la cola BLOCK porque pasará a EXIT!", entrenador->id);
			}
		}
	}
}

void team_planner_delete_from_new_queue(t_entrenador_pokemon* entrenador) {
	for (int i = 0; i < list_size(new_queue); i++) {
		t_entrenador_pokemon* entrenador_aux = list_get(new_queue, i);
		if (entrenador_aux->id == entrenador->id) {
			list_remove(new_queue, i);
			team_logger_info("Se eliminó al entrenador %d de la cola NEW porque pasará a READY. Fue seleccionado por el algoritmo de cercanía!", entrenador->id);
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
	entrenador->current_burst_time = 0;
	entrenador->total_burst_time = 0;
	entrenador->previus_burst = 0;
	entrenador->previus_estimation = (float)team_config->estimacion_inicial;
	entrenador->estimated_burst = 0;
	entrenador->status = true;
	entrenador->pokemon_a_atrapar = NULL;
	entrenador->deadlock = false;
	entrenador->list_id_catch = list_create();
	entrenador->diferencia = team_planner_calcular_diferencia(entrenador);
	entrenador->se_movio = false;
	pthread_mutex_init(&entrenador->sem_move_trainers, NULL);
	pthread_mutex_lock(&entrenador->sem_move_trainers);
	pthread_create(&entrenador->hilo_entrenador, NULL, (void*) move_trainers_and_catch_pokemon, entrenador);
	pthread_detach(entrenador->hilo_entrenador);

	return entrenador;
}

int team_planner_calcular_diferencia(t_entrenador_pokemon* entrenador) {
	int diferencia = 0;
	bool lo_tiene = false;
	t_pokemon* pokemon_default = malloc(sizeof(t_pokemon));
	t_pokemon* pokemon_objetivo = malloc(sizeof(t_pokemon));

	for (int i = 0; i < list_size(entrenador->pokemons); i++) {
		pokemon_default = list_get(entrenador->pokemons, i);

		for (int j = 0; j < list_size(entrenador->targets); j++) {
			pokemon_objetivo = list_get(entrenador->targets, i);
			if (!string_equals_ignore_case(pokemon_default->name, pokemon_objetivo->name)) {
				continue;
			} else {
				lo_tiene = true;
				break;
			}
		}

		if (!lo_tiene) {
			diferencia++;
		}
	}
	return diferencia;
}

t_pokemon* team_planner_pokemon_create(char* nombre) {
	t_pokemon* pokemon = malloc(sizeof(t_pokemon));
	pokemon->name = string_duplicate(nombre);
	return pokemon;
}

t_pokemon* team_planner_pokemon_appeared_create(char* nombre, int x, int y) {
	t_pokemon* pokemon = malloc(sizeof(t_pokemon));
	pokemon->name = nombre;
	t_position* pos = malloc(sizeof(t_position));
	pos->pos_x = x;
	pos->pos_y = y;
	pokemon->position = pos;
	return pokemon;
}

t_position* team_planner_extract_position(char* pos_spl) {
	char** splitted = string_split(pos_spl, split_char);
	t_position* posicion = malloc(sizeof(t_position));
	posicion->pos_x = atoi(utils_get_parameter_i(splitted, 0));
	posicion->pos_y = atoi(utils_get_parameter_i(splitted, 1));
	utils_free_array(splitted);

	return posicion;
}

void team_planner_extract_pokemons(t_list* pokemons, char* pokes_spl) {
	char** splitted = string_split(pokes_spl, split_char);
	int pokes_spl_size = utils_get_array_size(splitted);
	for (int j = 0; j < pokes_spl_size; j++) {
		char* nombre = string_duplicate(utils_get_parameter_i(splitted, j));
		t_pokemon* pokemon = team_planner_pokemon_create(nombre);
		list_add(pokemons, pokemon);
	}
	utils_free_array(splitted);
}

void team_planner_add_new_trainner(t_entrenador_pokemon* entrenador) {
	list_add(new_queue, entrenador);
	team_logger_info("Se añadió al entrenador %d a la cola NEW!", entrenador->id);
}

void team_planner_finish_trainner(t_entrenador_pokemon* entrenador) {
	entrenador->state = EXIT;
	entrenador->pokemon_a_atrapar = NULL;
	entrenador->deadlock = false;
	entrenador->se_movio = false;
	team_logger_info("El entrenador %d terminó exitosamente!", entrenador->id);
	team_planner_delete_from_bloqued_queue(entrenador, 1);
	list_add(exit_queue, entrenador);
}

t_entrenador_pokemon* team_planner_find_trainer_by_id_corr(uint32_t id) {
	for (int j = 0; j < list_size(block_queue); j++) {
		t_entrenador_pokemon* entrenador = list_get(block_queue, j);
		for (int i = 0; i < list_size(entrenador->list_id_catch); i++) {
			uint32_t id_corr = (uint32_t) list_get(entrenador->list_id_catch, i);
			if (id_corr == id) {
				return entrenador;
			}
		}
	}

	return NULL;
}

void team_planner_load_entrenadores() {
	int i = 0;

	bool next_pokemon_null = false;
	while (team_config->posiciones_entrenadores[i] != NULL) {
		t_position* posicion = team_planner_extract_position(team_config->posiciones_entrenadores[i]);
		t_list* pokemons = list_create();
		if (!next_pokemon_null && !string_equals_ignore_case(utils_array_to_string(team_config->pokemon_entrenadores), "[]") &&
				team_config->pokemon_entrenadores[i] != NULL) {
			if(team_config->pokemon_entrenadores[i + 1] == NULL) {
				next_pokemon_null = true;
			}
			team_planner_extract_pokemons(pokemons, team_config->pokemon_entrenadores[i]);
		}
		t_list* objetivos = list_create();
		if (!string_equals_ignore_case(utils_array_to_string(team_config->objetivos_entrenadores), "[]") &&
				team_config->objetivos_entrenadores[i] != NULL &&
				!string_equals_ignore_case(team_config->objetivos_entrenadores[i], "")) {
			team_planner_extract_pokemons(objetivos, team_config->objetivos_entrenadores[i]);
		}

		t_entrenador_pokemon* entrenador = team_planner_entrenador_create(i, posicion, pokemons, objetivos);
		team_planner_add_new_trainner(entrenador);
		list_add_all(total_targets_pokemons, objetivos);
		list_add_all(got_pokemons, pokemons);
		i++;
	}

	team_logger_info("Hay %d entrenadores en la cola de NEW.", list_size(new_queue));

	team_planner_get_real_targets();
	sem_post(&sem_pokemons_to_get);

	for (int i = 0; i < list_size(new_queue); i++) {
		sem_post(&sem_entrenadores_disponibles);
	}
}

void team_planner_get_real_targets() {
	t_list* aux = list_create();
	aux = total_targets_pokemons;

	for (int i = 0; i < list_size(aux); i++) {
		t_pokemon* goal = list_get(aux, i);

		for (int j = 0; j < list_size(got_pokemons); j++) {
			t_pokemon* got = list_get(got_pokemons, j);

			if (string_equals_ignore_case(got->name, goal->name)) {
				list_remove(aux, i);
				i++;
				break;
			}
		}
	}
	real_targets_pokemons = aux;
}

void planner_init_quees() {
	new_queue = list_create();
	ready_queue = list_create();
	block_queue = list_create();
	exit_queue = list_create();
	pokemon_to_catch = list_create();
	message_catch_sended = list_create();
	pokemones_pendientes = list_create();
	real_targets_pokemons = list_create();
	lista_auxiliar = list_create();
	pokemons_localized = list_create();
	get_id_corr = list_create();
	exec_queue = list_create();
	total_targets_pokemons = list_create();
	got_pokemons = list_create();
}

int team_planner_get_least_estimate_index() {
	int least_index = 0;

	float aux_estimated = 1000.0f;
	if(list_size(ready_queue) != 0){
		for (int i = 0; i < list_size(ready_queue); i++) {
			t_entrenador_pokemon* entrenador = list_get(ready_queue, i);
			float estimated = team_planner_calculate_exponential_mean(entrenador);

			if (estimated < aux_estimated) {
				aux_estimated = estimated;
				least_index = i;
			}
		}
	}
	return least_index;
}

void team_planner_new_cpu_cicle(t_entrenador_pokemon* entrenador) {
	entrenador->current_burst_time++;
	entrenador->total_burst_time++;
	entrenador->estimated_burst--;

	if (team_config->algoritmo_planificacion == RR && !entrenador->deadlock) {
		team_planner_check_RR_burst(entrenador); 
	} 

	if (team_config->algoritmo_planificacion == SJF_CD && !entrenador->deadlock) {
		team_planner_check_SJF_CD_time(entrenador); 
	}
}

float team_planner_calculate_exponential_mean(t_entrenador_pokemon* entrenador) {
	float alpha = team_config->alpha;
	float next_tn = (alpha * (float) entrenador->previus_estimation) + ((1.0 - alpha) * entrenador->previus_burst);
	return next_tn;
}

t_entrenador_pokemon* team_planner_exec_trainer(t_entrenador_pokemon* entrenador) {
	entrenador->current_burst_time = 0;
	entrenador->state = EXEC;
	if(entrenador->se_movio){
		entrenador->se_movio = false;
	}

	return entrenador;
}

bool team_planner_is_SJF_algorithm() {
	return team_config->algoritmo_planificacion == SJF_CD || team_config->algoritmo_planificacion == SJF_SD;
}

bool _is_available(t_entrenador_pokemon* trainner) {
	return trainner->status && !trainner->deadlock;
}

t_list* filter_block_list_by_0() {
	return list_filter(block_queue, (void*) _is_available);
}

t_list* team_planner_filter_by_deadlock() {
	bool _is_locked(t_entrenador_pokemon* trainner) {
		return trainner->deadlock;
	}
	return list_filter(block_queue, (void*) _is_locked);
}

t_list* team_planner_create_ready_queue() {	
	t_list* bloquados_en_cero = filter_block_list_by_0(block_queue, (void*) _is_available);
	
	t_list* listo_para_planificar = list_create();
	list_add_all(listo_para_planificar, bloquados_en_cero);
	list_add_all(listo_para_planificar, new_queue);

	return listo_para_planificar;
}

bool _is_waiting(t_entrenador_pokemon* trainner) {
	return !trainner->status;
}

t_list* filter_block_list_by_1() {
	return list_filter(block_queue, (void*) _is_waiting);
}

t_list* team_planner_trainers_waiting_messages() {	
	return filter_block_list_by_1(block_queue, (void*) _is_waiting);
}

t_entrenador_pokemon* team_planner_apply_SJF() {
	int least_estimate_index = team_planner_get_least_estimate_index();	
	t_entrenador_pokemon* entrenador = list_get(ready_queue, least_estimate_index);
	entrenador->estimated_burst = team_planner_calculate_exponential_mean(entrenador);
	list_remove(ready_queue, least_estimate_index);	
	team_logger_info("Se eliminó al entrenador %d de la cola de READY porque es su turno de ejecutar.", entrenador->id);
	
	return team_planner_exec_trainer(entrenador);	
}

t_entrenador_pokemon* team_planner_apply_FIFO() {
	t_entrenador_pokemon* entrenador;
	entrenador = list_get(ready_queue, 0);
	list_remove(ready_queue, 0);
	team_logger_info("Se eliminó al entrenador %d de la cola de READY porque es su turno de ejecutar.", entrenador->id);

	return team_planner_exec_trainer(entrenador);
}

t_entrenador_pokemon* team_planner_apply_RR() {
	t_entrenador_pokemon* entrenador;
	entrenador = list_get(ready_queue, 0);
	list_remove(ready_queue, 0);
	team_logger_info("Se eliminó al entrenador %d de la cola de READY porque es su turno de ejecutar.", entrenador->id);

	return team_planner_exec_trainer(entrenador);	
}

t_entrenador_pokemon* team_planner_set_algorithm() {
	switch (team_config->algoritmo_planificacion) {
		case FIFO:
			return team_planner_apply_FIFO();
			break;
		case RR:
			return team_planner_apply_RR();
			break;
		case SJF_CD:
			return team_planner_apply_SJF();
			break;
		case SJF_SD:
			return team_planner_apply_SJF();
			break;
	}
	return NULL;
}

bool team_planner_all_queues_are_empty_except_block() {
	int bloqueados_en_deadlock = list_size(team_planner_filter_by_deadlock());
	int bloqueados = list_size(block_queue);

	return (bloqueados_en_deadlock == bloqueados) && list_is_empty(new_queue) && list_is_empty(exec_queue) && list_is_empty(filter_block_list_by_1()) && list_is_empty(ready_queue) && (list_size(exit_queue) != list_size(team_planner_get_trainners()));
}

void team_planner_solve_deadlock() {

	team_logger_info("Se iniciará la detección y resolución de DEADLOCKS!");
	int a = 0;
	int quantum = 0;

	t_pokemon* pokemon_de_entrenador_bloqueado;
	t_pokemon* pokemon_de_entrenador_bloqueante;
	t_entrenador_pokemon* entrenador_bloqueado;

	while (list_size(block_queue) > 0) {
		t_entrenador_pokemon* entrenador_bloqueante = list_get(block_queue, a);

		pokemon_de_entrenador_bloqueante = team_planner_ver_a_quien_no_necesita(entrenador_bloqueante, entrenador_bloqueante->pokemons);

		entrenador_bloqueado = team_planner_entrenador_que_necesita(pokemon_de_entrenador_bloqueante);

		entrenador_bloqueado->pokemon_a_atrapar = malloc(sizeof(t_pokemon));
		entrenador_bloqueado->pokemon_a_atrapar->name = string_duplicate(pokemon_de_entrenador_bloqueante->name);
		entrenador_bloqueado->pokemon_a_atrapar->position = malloc(sizeof(t_position));
		entrenador_bloqueado->pokemon_a_atrapar->position->pos_x = entrenador_bloqueante->position->pos_x;
		entrenador_bloqueado->pokemon_a_atrapar->position->pos_y = entrenador_bloqueante->position->pos_y;

		deadlocks_detected++;	

		team_logger_info("Se detectó un DEADLOCK. El entrenador %d está bloqueando al entrenador %d.", entrenador_bloqueante->id, entrenador_bloqueado->id);

		team_planner_add_to_ready_queue(entrenador_bloqueado);
		list_remove(ready_queue, 0);
		team_planner_exec_trainer(entrenador_bloqueado);

		team_logger_info("El entrenador %d pasará a estado EXEC para realizar el INTERCAMBIO con el entrenador bloqueante %d!", entrenador_bloqueado-> id, entrenador_bloqueante->id);

		int aux_x = entrenador_bloqueado->position->pos_x - entrenador_bloqueado->pokemon_a_atrapar->position->pos_x;
		int	aux_y = entrenador_bloqueado->position->pos_y - entrenador_bloqueado->pokemon_a_atrapar->position->pos_y;

		int steps = fabs(aux_x) + fabs(aux_y);

		context_switch_qty++;

		for (int i = 0; i < steps; i++) {
			sleep(team_config->retardo_ciclo_cpu);
			team_planner_new_cpu_cicle(entrenador_bloqueado);

			if(team_config->algoritmo_planificacion == RR){
				if(quantum == team_config->quantum){
					quantum = 0;
					team_logger_info("El entrenador %d pasó a la cola de READY ya que terminó su QUANTUM.", entrenador_bloqueado->id);
					usleep(500);
					context_switch_qty++;
					team_logger_info("El entrenador %d pasará a estado EXEC para realizar el INTERCAMBIO con el entrenador bloqueante %d!", entrenador_bloqueado-> id, entrenador_bloqueante->id);
				}
				quantum++;
			}
		}

		team_logger_info("El entrenador %d se movió de (%d, %d) a (%d, %d)", entrenador_bloqueado->id,
																			 entrenador_bloqueado->position->pos_x,
																			 entrenador_bloqueado->position->pos_y,
																		  	 entrenador_bloqueado->pokemon_a_atrapar->position->pos_x,
																			 entrenador_bloqueado->pokemon_a_atrapar->position->pos_y);

		entrenador_bloqueado->position->pos_x = entrenador_bloqueado->pokemon_a_atrapar->position->pos_x;
		entrenador_bloqueado->position->pos_y = entrenador_bloqueado->pokemon_a_atrapar->position->pos_y;

		list_add(block_queue, entrenador_bloqueado);
		entrenador_bloqueado->status = false;

		pokemon_de_entrenador_bloqueado = team_planner_ver_a_quien_no_necesita(entrenador_bloqueado, entrenador_bloqueado->pokemons); ///LO TRAE VACIO

		for (int i = 0; i < 5; i++) {
			sleep(team_config->retardo_ciclo_cpu);
			entrenador_bloqueado->total_burst_time++;
		}

		team_logger_info("Se añadió al entrenador %d a la cola de bloqueados luego de ejecutar un intercambio.", entrenador_bloqueado->id);

		list_add(entrenador_bloqueado->pokemons, pokemon_de_entrenador_bloqueante);
		team_logger_info("El entrenador %d ahora tiene un %s que intercambió con el entrenador %d!", entrenador_bloqueado->id, pokemon_de_entrenador_bloqueante->name, entrenador_bloqueante->id);

		list_add(entrenador_bloqueante->pokemons, pokemon_de_entrenador_bloqueado);
		team_logger_info("El entrenador %d ahora tiene un %s que intercambió con el entrenador %d!", entrenador_bloqueante->id, pokemon_de_entrenador_bloqueado->name, entrenador_bloqueado->id);

		team_planner_remove_from_pokemons_list(entrenador_bloqueado, pokemon_de_entrenador_bloqueado);
		team_planner_remove_from_pokemons_list(entrenador_bloqueante, pokemon_de_entrenador_bloqueante);	

		if (team_planner_trainer_completed_with_success(entrenador_bloqueado)) {
			team_planner_finish_trainner(entrenador_bloqueado);
		}

		if (team_planner_trainer_completed_with_success(entrenador_bloqueante)) {
			team_planner_finish_trainner(entrenador_bloqueante);
		}

		deadlocks_resolved++;
	}
	if (team_planner_all_finished()) {
		team_logger_info("Finaliza el algoritmo de detección de interbloqueos. El TEAM se encuentra en condiciones de FINALIZAR!");
		team_planner_print_fullfill_target();
		team_planner_exit();
		socket_close_conection(team_socket);
		exit(0);
	}
}

void team_planner_exit() {
	team_config_free();
	team_logger_destroy();
	team_planner_destroy();
}

void team_planner_remove_from_pokemons_list(t_entrenador_pokemon* entrenador, t_pokemon* pokemon) {
	for (int i = 0; i < list_size(entrenador->pokemons); i++) {
		t_pokemon* pokemon_aux = list_get(entrenador->pokemons, i);

		if (string_equals_ignore_case(pokemon->name, pokemon_aux->name)) {
			list_remove(entrenador->pokemons, i);
			break;
		}
	}
}

void team_planner_eliminar_pokemon_de_objetivos(t_list* list, char* nombre) {
	for (int i = 0; i < list_size(list); i++) {
		t_pokemon* pok = list_get(list, i);
		if (string_equals_ignore_case(pok->name, nombre)) {
			list_remove(list, i);
			break;
		}
	}
}

t_entrenador_pokemon* team_planner_entrenador_que_necesita(t_pokemon* pokemon_de_entrenador_bloqueado) {
	int contador = 0;
	bool _necesitan_pokemon_bloquante(t_entrenador_pokemon* trainner) {
		contador = 0;
		for (int i = 0; i < list_size(trainner->targets); i++) {
			t_pokemon* pokemon_objetivo = list_get(trainner->targets, i);
			if (!string_equals_ignore_case(pokemon_objetivo->name, pokemon_de_entrenador_bloqueado->name)) {
				continue;
			} else {
				contador++;
			}
		}
		return contador > 0;
	}
	t_list* entrenadores_pokemon_bloqueante = list_filter(block_queue, (void*) _necesitan_pokemon_bloquante);

	int _entrenador_no_tiene_pokemon_bloqueante(t_entrenador_pokemon *trainner) {

		int cantidad_repetidos = 0;
		for (int i = 0; i < list_size(trainner->targets); i++) {
			t_pokemon* pokemon_objetivo = list_get(trainner->targets, i);
			if (!string_equals_ignore_case(pokemon_objetivo->name, pokemon_de_entrenador_bloqueado->name)) {
				continue;
			} else {
				cantidad_repetidos++;
			}
		}

		for (int i = 0; i < list_size(trainner->pokemons); i++) {
			t_pokemon* pokemon_obtenido = list_get(trainner->pokemons, i);
			if (string_equals_ignore_case(pokemon_obtenido->name, pokemon_de_entrenador_bloqueado->name)) {
				cantidad_repetidos--;
				continue;
			}
		}
		return cantidad_repetidos > 0 ? 1 : 0;
	}

	return list_find(entrenadores_pokemon_bloqueante, (void*) _entrenador_no_tiene_pokemon_bloqueante);
}

t_pokemon* team_planner_ver_a_quien_no_necesita(t_entrenador_pokemon* entrenador, t_list* pokemons_que_tiene) {
	bool le_sirve = true;
	t_pokemon* pokemon_a_entregar;
	list_clean(lista_auxiliar);
	lista_auxiliar = list_duplicate(entrenador->targets);
	int m = 0;

	while (m < list_size(pokemons_que_tiene)) {
		for (int i = 0; i < list_size(pokemons_que_tiene); i++) {
			pokemon_a_entregar = list_get(pokemons_que_tiene, i);

			for (int j = 0; j < list_size(entrenador->targets); j++) {
				t_pokemon* pokemon_objetivo = list_get(entrenador->targets, j);
				if (string_equals_ignore_case(pokemon_objetivo->name, pokemon_a_entregar->name)) {
					team_planner_eliminar_pokemon_de_objetivos(lista_auxiliar, pokemon_a_entregar->name);
					le_sirve = true;
					break;
				} else {
					le_sirve = false;
				}
			}
		
			if (!le_sirve) {
				break;
			}
		}

		if (!list_is_empty(lista_auxiliar)) {
			if (team_planner_entrenador_que_necesita(pokemon_a_entregar) != NULL) {
				return pokemon_a_entregar;
			}
		}
		list_clean(lista_auxiliar);
		m++;

	}
	return NULL;
}

t_list* team_planner_remover_de_lista (t_list* lista, t_pokemon* pokemon) {
	for (int i = 0; i < list_size(lista); i++) {
		t_pokemon* pok = list_get(lista, i);
		if (string_equals_ignore_case(pok->name, pokemon->name)) {
			list_remove(lista, i);
			break;
		}
	}
	return lista;
}

t_list* team_planner_get_trainners() {
	t_list* trainners = list_create();
	list_add_all(trainners, new_queue);
	list_add_all(trainners, ready_queue);
	list_add_all(trainners, block_queue);
	list_add_all(trainners, exit_queue);
	list_add_all(trainners, exec_queue);

	return trainners;
}

void team_planner_print_fullfill_target() {
	t_list* trainners = team_planner_get_trainners();

	int add_burst_time(int accum, t_entrenador_pokemon* trainner) {
      return accum + trainner->total_burst_time;
	}
	int total_cpu = (int) list_fold(trainners, 0, (void*) add_burst_time);
	team_logger_info("Cantidad de ciclos de CPU totales: %d.", total_cpu);
	team_logger_info("Cantidad de cambios de contexto realizados: %d.", context_switch_qty);
	team_logger_info("Cantidad de ciclos de CPU realizados por entrenador:");
	void _list_burst_trainner(t_entrenador_pokemon *trainner) {
		team_logger_info("Entrenador %d -> Ciclos de CPU realizados: %d", trainner->id, trainner->total_burst_time);
	}
	list_iterate(trainners, (void*) _list_burst_trainner);

	list_destroy(trainners);
	team_logger_info("Deadlocks producidos: %d  y resueltos: %d.", deadlocks_detected, deadlocks_resolved);
}

void team_planner_init() {
	planner_init_quees();
	team_planner_load_entrenadores();	
}

bool team_planner_trainer_completed_with_success(t_entrenador_pokemon* entrenador) {
	list_clean(lista_auxiliar);
	lista_auxiliar = list_duplicate(entrenador->targets);
	if (list_size(entrenador->pokemons) == list_size(entrenador->targets) || (list_size(entrenador->pokemons)- list_size(entrenador->targets) == entrenador->diferencia)) {

		for (int i = 0; i < list_size(entrenador->pokemons); i++) {
			t_pokemon* pokemon_obtenido = list_get(entrenador->pokemons, i);

			for (int j = 0; j < list_size(lista_auxiliar); j++) {
				t_pokemon* pokemon_objetivo = list_get(lista_auxiliar, j);
				if (string_equals_ignore_case(pokemon_objetivo->name, pokemon_obtenido->name)) {
					list_remove(lista_auxiliar, j);
				}
			}
		}
		if (entrenador->diferencia == 0) {
			return list_is_empty(lista_auxiliar);
		} else {
			return list_size(lista_auxiliar) == entrenador->diferencia;
		}
	}
	list_clean(lista_auxiliar);
	return false;
}

void team_planner_destroy_pokemons(t_pokemon* pokemon) {
	if (pokemon != NULL) {
		free(pokemon->name);
	}
}

void team_planner_destroy_entrenador(t_entrenador_pokemon* entrenador) {
	if (entrenador != NULL) {
		if (&entrenador->sem_move_trainers != NULL) {
			pthread_mutex_destroy(&entrenador->sem_move_trainers);
		}

		if (!list_is_empty(entrenador->list_id_catch)) {
			list_destroy(entrenador->list_id_catch);
		}
	}
}

void planner_destroy_quees() {

	list_destroy(message_catch_sended);
	list_destroy(pokemones_pendientes);
	list_destroy(lista_auxiliar);
	list_destroy(get_id_corr);
	list_destroy(pokemons_localized);
	list_destroy(got_pokemons);
	list_destroy(pokemon_to_catch);

	if (!list_is_empty(new_queue)) {
		list_destroy_and_destroy_elements(new_queue, (void*) team_planner_destroy_entrenador);
	}

	if (!list_is_empty(ready_queue)) {
		list_destroy_and_destroy_elements(ready_queue, (void*) team_planner_destroy_entrenador);
	}

	if (!list_is_empty(block_queue)) {
		list_destroy_and_destroy_elements(block_queue, (void*) team_planner_destroy_entrenador);
	}

	if (!list_is_empty(exit_queue)) {
		list_destroy_and_destroy_elements(exit_queue, (void*) team_planner_destroy_entrenador);
	}

	if (!list_is_empty(exec_queue)) {
		list_destroy_and_destroy_elements(exec_queue, (void*) team_planner_destroy_entrenador);
	}

	if (!list_is_empty(total_targets_pokemons)) {
		list_destroy_and_destroy_elements(total_targets_pokemons, (void*) team_planner_destroy_pokemons);
	}
}

void team_planner_destroy() {
	sem_destroy(&sem_entrenadores_disponibles);
	sem_destroy(&sem_message_on_queue);
	sem_destroy(&sem_trainers_in_ready_queue);
	sem_destroy(&sem_pokemons_to_get);
	sem_destroy(&sem_planificador);
	pthread_mutex_destroy(&cola_pokemons_a_atrapar);
	pthread_mutex_destroy(&cola_exec);
	planner_destroy_quees();
}
