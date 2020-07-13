#include "team.h"

int main(int argc, char *argv[]) {
	if (team_load() < 0)
		return EXIT_FAILURE;

	team_init();

	return EXIT_SUCCESS;
}

int team_load() {
	int response = team_config_load();
	if (response < 0)
		return response;

	response = team_logger_create(team_config->log_file);
	if (response < 0) {
		team_config_free();
		return response;
	}
	team_print_config();

	return 0;
}

void team_init() {
	planificacion = true;
	cercania = true;
	sem_init(&sem_entrenadores_disponibles, 0, 0);
	sem_init(&sem_pokemons_to_get, 0, 1);
	sem_init(&sem_message_on_queue, 0, 0);
	sem_init(&sem_planificador, 0, 1);
	sem_init(&sem_trainers_in_ready_queue, 0, 0);
	sem_init(&sem_deadlock, 0, 0);
	pthread_mutex_init(&cola_pokemons_a_atrapar, NULL);
	planificador = true;
	cercania = true;
	message_catch_sended = list_create();
	pokemones_pendientes = list_create();
	real_targets_pokemons = list_create();
	lista_auxiliar = list_create();
	pokemons_localized = list_create();
	get_id_corr = list_create();
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
	pthread_t tid1;
	pthread_t tid2;
	pthread_t tid3;
	pthread_t planificator;
	pthread_t algoritmo_cercania_entrenadores;
	team_planner_init();
	send_get_message();

	t_cola cola_appeared = APPEARED_QUEUE;
	pthread_create(&tid1, NULL, (void*) team_retry_connect,
			(void*) &cola_appeared);
	pthread_detach(tid1);

	t_cola cola_localized = LOCALIZED_QUEUE;
	pthread_create(&tid2, NULL, (void*) team_retry_connect,
			(void*) &cola_localized);
	pthread_detach(tid2);

	t_cola cola_caught = CAUGHT_QUEUE;
	pthread_create(&tid3, NULL, (void*) team_retry_connect,
			(void*) &cola_caught);
	pthread_detach(tid3);

	pthread_create(&algoritmo_cercania_entrenadores, NULL,
			(void*) team_planner_algoritmo_cercania, NULL);
	pthread_detach(algoritmo_cercania_entrenadores);

	pthread_create(&planificator, NULL, (void*) team_planner_run_planification,
	NULL);
	pthread_detach(planificator);

	team_server_init();
	usleep(500000);
}

void remove_pokemon_from_catch(t_pokemon* pokemon) {
	for (int i = 0; i < list_size(pokemon_to_catch); i++) {
		t_pokemon_received* pokemon_con_posiciones = list_get(pokemon_to_catch,
				i);

		if (string_equals_ignore_case(pokemon_con_posiciones->name,
				pokemon->name)) {
			t_list* posiciones_pokemon = list_create();
			posiciones_pokemon = pokemon_con_posiciones->pos;

			if (list_size(posiciones_pokemon) == 1) {
				pthread_mutex_lock(&cola_pokemons_a_atrapar);
				list_remove(pokemon_to_catch, i);
				pthread_mutex_unlock(&cola_pokemons_a_atrapar);
			} else {

				for (int j = 0; j < list_size(posiciones_pokemon); j++) {
					t_position* position = list_get(posiciones_pokemon, j);
					if (position->pos_x == pokemon->position->pos_x
							&& position->pos_y == pokemon->position->pos_x) {
						pthread_mutex_lock(&cola_pokemons_a_atrapar);
						list_remove(pokemon_con_posiciones->pos, j);
						pthread_mutex_unlock(&cola_pokemons_a_atrapar);
					}
				}
			}
		}
	}
}

void send_message_catch(t_catch_pokemon* catch_send,
		t_entrenador_pokemon* entrenador) {
	t_protocol catch_protocol = CATCH_POKEMON;

	entrenador->state = BLOCK;
	list_add(block_queue, entrenador);
	team_logger_info(
			"Se añadió al entrenador %d a la cola de BLOCK luego de intentar un CATCH a la espera de confirmación",
			entrenador->id);

	int i = send_message(catch_send, catch_protocol, NULL);

	if (i == 0) {
		team_planner_change_block_status_by_id_corr(1,
				catch_send->id_correlacional);
		list_add(message_catch_sended, catch_send);
		list_add(entrenador->list_id_catch,
				(void*) catch_send->id_correlacional);
	} else {
		atrapar_pokemon(entrenador, catch_send->nombre_pokemon);
	}
	usleep(500000);
}

bool todavia_quedan_pokemones_restantes(char* tipo) {
	for (int i = 0; i < list_size(real_targets_pokemons); i++) {
		t_pokemon* pokemon = list_get(real_targets_pokemons, i);
		if (string_equals_ignore_case(pokemon->name, tipo)) {
			return true;
		}
	}
	return false;
}

void remover_totalmente_de_pokemon_to_catch(char* tipo) {
	for (int i = 0; i < list_size(pokemon_to_catch); i++) {
		t_pokemon_received* pokemon = list_get(pokemon_to_catch, i);
		if (string_equals_ignore_case(pokemon->name, tipo)) {
			list_remove(pokemon_to_catch, i);
		}
	}
}

bool tengo_en_pokemon_to_catch(char* tipo) {
	for (int i = 0; i < list_size(pokemon_to_catch); i++) {
		t_pokemon_received* pokemon = list_get(pokemon_to_catch, i);
		if (string_equals_ignore_case(pokemon->name, tipo)) {
			return true;
		}
	}
	return false;
}

void atrapar_pokemon(t_entrenador_pokemon* entrenador, char* pokemon_name) {
	team_planner_change_block_status_by_trainer(0, entrenador);
	t_pokemon* pokemon = team_planner_pokemon_create(pokemon_name);
	list_add(entrenador->pokemons, pokemon);
	team_logger_info("El entrenador %d atrapó un %s en la posicion (%d, %d)!!",
			entrenador->id, pokemon_name,
			entrenador->pokemon_a_atrapar->position->pos_x,
			entrenador->pokemon_a_atrapar->position->pos_y);
	quitar_de_pokemones_pendientes(pokemon_name);
	quitar_de_real_target(pokemon_name);

	if (todavia_quedan_pokemones_restantes(pokemon_name)) { //me fijo si sigo necesitando ese tipo de pokemon, puede ser que en un localized tenga una posición extra
		if (tengo_en_pokemon_to_catch(pokemon_name)) {
			sem_post(&sem_message_on_queue); //activa al algoritmo de cercanía
		}
	} else { //no necesito más de ese tipo de pokemon
		if (tengo_en_pokemon_to_catch(pokemon_name)) { //si sigo teniendo posiciones, las borro
			remover_totalmente_de_pokemon_to_catch(pokemon_name);
		}
	}

	if (trainer_completed_with_success(entrenador)) {
		team_planner_finish_trainner(entrenador);
	}

	if (trainer_is_in_deadlock_caught(entrenador)) {
		entrenador->deadlock = true;
	}

	if (all_queues_are_empty_except_block()) { // TODO no entra resolver deadlock
		solve_deadlock();
	}

	if (all_finished()) { //TODO: no finaliza, controlar que termine con exito
		cercania = false;
		planificacion = false;
		pthread_cancel(algoritmo_cercania_entrenadores);
		pthread_cancel(planificator);

		team_planner_end_trainer_threads();
	}
}

bool all_finished() {
	int all = list_size(team_planner_get_trainners());
	int exit = list_size(exit_queue);
	return all == exit;
}

bool pokemon_not_pendant(char* pokemon) {
	for (int i = 0; i < list_size(pokemones_pendientes); i++) {
		char* nombre = list_get(pokemones_pendientes, i);

		if (string_equals_ignore_case(nombre, pokemon)) {
			return false;
		}
	}
	return true;
}

void send_get_message() {
	sem_wait(&sem_pokemons_to_get);
	t_protocol get_protocol;
	t_get_pokemon* get_send = malloc(sizeof(t_get_pokemon));

	for (int i = 0; i < list_size(real_targets_pokemons); i++) {
		t_pokemon* pk = list_get(real_targets_pokemons, i);
		get_send->id_correlacional = 0;
		get_send->nombre_pokemon = string_duplicate(pk->name);
		get_send->tamanio_nombre = strlen(get_send->nombre_pokemon) + 1;

		get_protocol = GET_POKEMON;
		if ((send_message(get_send, get_protocol, get_id_corr)) > 0) {
			team_logger_info("Se recibió id correlacional %d en respuesta al GET", get_id_corr);
		}
		usleep(500000);
	}
}

int send_message(void* paquete, t_protocol protocolo, t_list* queue) {
	int broker_fd_send = socket_connect_to_server(team_config->ip_broker, team_config->puerto_broker);

	if (broker_fd_send < 0) {
		team_logger_warn("No se pudo conectar con BROKER.");
		socket_close_conection(broker_fd_send);
		return -1;
	} else {
		team_logger_info("Conexión con BROKER establecida correctamente!");
		utils_serialize_and_send(broker_fd_send, protocolo, paquete);

		uint32_t id_corr;
		int recibido = recv(broker_fd_send, &id_corr, sizeof(uint32_t), MSG_WAITALL);
		if (recibido > 0 && queue != NULL) {
			list_add(queue, (void*) id_corr);
		}
		if (protocolo == CATCH_POKEMON) {
			t_catch_pokemon *catch_send = (t_catch_pokemon*) paquete;
			catch_send->id_correlacional = id_corr;
		}
		socket_close_conection(broker_fd_send);
	}
	return 0;
}

void check_RR_burst(t_entrenador_pokemon* entrenador) {
	if (entrenador->current_burst_time == team_config->quantum) {
		add_to_ready_queue(entrenador);
		sem_post(&sem_trainers_in_ready_queue);
		sem_post(&sem_planificador);
		team_logger_info("Se añadió al entrenador %d a la cola de READY ya que terminó su QUANTUM", entrenador->id);
		pthread_mutex_lock(&entrenador->sem_move_trainers);
	}
}

void check_SJF_CD_time(t_entrenador_pokemon* entrenador) {
	if (entrenador->estimated_time > team_planner_get_least_estimate_index()) {
		add_to_ready_queue(entrenador);
		sem_post(&sem_trainers_in_ready_queue);
		sem_post(&sem_planificador);
		team_logger_info("Se añadió al entrenador %d a la cola de READY ya que será desalojado por otro con ráfaga más corta", entrenador->id);
		pthread_mutex_lock(&entrenador->sem_move_trainers);
	}
}

void move_trainers_and_catch_pokemon(t_entrenador_pokemon* entrenador) {
	while (entrenador->esta_activo) {
		pthread_mutex_lock(&entrenador->sem_move_trainers);
		int aux_x = entrenador->position->pos_x - entrenador->pokemon_a_atrapar->position->pos_x;
		int aux_y = entrenador->position->pos_y - entrenador->pokemon_a_atrapar->position->pos_y;

		int steps = fabs(aux_x + aux_y);

		for (int i = 0; i <= steps; i++) {
			sleep(team_config->retardo_ciclo_cpu);
			new_cpu_cicle(entrenador);
			entrenador->current_burst_time++;
			entrenador->total_burst_time++;
		}

		team_logger_info("El entrenador %d se movió de (%d, %d) a (%d, %d)",
				entrenador->id, entrenador->position->pos_x,
				entrenador->position->pos_y,
				entrenador->pokemon_a_atrapar->position->pos_x,
				entrenador->pokemon_a_atrapar->position->pos_y);

		entrenador->position->pos_x = entrenador->pokemon_a_atrapar->position->pos_x;
		entrenador->position->pos_y = entrenador->pokemon_a_atrapar->position->pos_y;

		if (entrenador->blocked_info == NULL && !entrenador->deadlock) {
			t_catch_pokemon* catch_send = malloc(sizeof(t_catch_pokemon));
			catch_send->id_correlacional = 0;
			catch_send->nombre_pokemon = entrenador->pokemon_a_atrapar->name;
			catch_send->pos_x = entrenador->pokemon_a_atrapar->position->pos_x;
			catch_send->pos_y = entrenador->pokemon_a_atrapar->position->pos_y;
			catch_send->tamanio_nombre = strlen(catch_send->nombre_pokemon);
			send_message_catch(catch_send, entrenador);
			sem_post(&sem_planificador);
			entrenador->pokemon_a_atrapar = NULL;
		}
	}
	pthread_exit(0);
}

void subscribe_to(void *arg) {

	t_cola cola = *((int *) arg);

	int new_broker_fd = socket_connect_to_server(team_config->ip_broker, team_config->puerto_broker);

	if (new_broker_fd < 0) {
		team_logger_warn("No se pudo conectar con BROKER.");
		socket_close_conection(new_broker_fd);
	} else {
		team_logger_info("Conexión con BROKER establecida correctamente!");
		t_subscribe* sub_snd = malloc(sizeof(t_subscribe));

		t_protocol subscribe_protocol = SUBSCRIBE;
		sub_snd->ip = string_duplicate(team_config->ip_team);
		sub_snd->puerto = team_config->puerto_team;
		sub_snd->proceso = TEAM;
		sub_snd->cola = cola;
		utils_serialize_and_send(new_broker_fd, subscribe_protocol, sub_snd);

		receive_msg(new_broker_fd, 0);
		is_connected = true;
	}
}

void team_retry_connect(void* arg) {
	void* arg2 = arg;
	while (true) {
		is_connected = false;
		subscribe_to(arg2);
		utils_delay(team_config->tiempo_reconexion);
	}
}

t_catch_pokemon* filter_msg_catch_by_id_caught(uint32_t id_corr_caught) {
	for (int i = 0; i < list_size(message_catch_sended); i++) {
		t_catch_pokemon* catch_message = list_get(message_catch_sended, i);

		if (catch_message->id_correlacional == id_corr_caught) {
			return catch_message;
		}
	}
	return NULL;
}

t_entrenador_pokemon* filter_trainer_by_id_caught(uint32_t id_corr_caught) {
	for (int i = 0; i < list_size(block_queue); i++) {
		t_entrenador_pokemon* entrenador = list_get(block_queue, i);
		for (int j = 0; j < list_size(entrenador->list_id_catch); j++) {
			uint32_t id_aux = (uint32_t) list_get(entrenador->list_id_catch, j);
			if (id_aux == id_corr_caught) {
				return entrenador;
			}
		}
	}
	return NULL;
}

void *receive_msg(int fd, int send_to) {
	int protocol;
	int is_server = send_to;

	while (true) {
		int received_bytes = recv(fd, &protocol, sizeof(int), 0);
		if (received_bytes <= 0) {
			team_logger_error("Se perdió la conexión con el socket %d.", fd);
			return NULL;
		}

		switch (protocol) {

		case CAUGHT_POKEMON: {
			t_caught_pokemon *caught_rcv = utils_receive_and_deserialize(fd, protocol);
			team_logger_info("Caught received ID correlacional: %d Resultado (0/1): %d", caught_rcv->id_correlacional, caught_rcv->result);
			usleep(50000);

			if (is_server == 0) {
				t_protocol ack_new = ACK;
				int r = send(fd, &ack_new, sizeof(t_protocol), 0);
				team_logger_warn("%d", r);
				team_logger_warn("ACK sent");
			}

			t_catch_pokemon* catch_message = filter_msg_catch_by_id_caught(caught_rcv->id_correlacional);
			t_entrenador_pokemon* entrenador = filter_trainer_by_id_caught(caught_rcv->id_correlacional);

			team_planner_change_block_status_by_id_corr(0, caught_rcv->id_correlacional);
			quitar_de_pokemones_pendientes(catch_message->nombre_pokemon);

			if (caught_rcv->result) {
				list_add(entrenador->pokemons, catch_message->nombre_pokemon);
				atrapar_pokemon(entrenador, catch_message->nombre_pokemon);

			}
			break;
		}

		case LOCALIZED_POKEMON: { //TODO no busca el pokemon cuando es valido
			t_localized_pokemon *loc_rcv = utils_receive_and_deserialize(fd, protocol);
			team_logger_info("Localized received! ID correlacional: %d Nombre Pokemon: %s Largo nombre: %d Cant elementos en lista: %d",
					loc_rcv->id_correlacional, loc_rcv->nombre_pokemon, loc_rcv->tamanio_nombre, loc_rcv->cant_elem);
			if (loc_rcv->cant_elem > 0) {

				usleep(500000);

				if (is_server == 0) {
					t_protocol ack_new = ACK;
					int r = send(fd, &ack_new, sizeof(t_protocol), 0);
					team_logger_warn("%d", r);
					team_logger_warn("ACK sent");
				}

				bool _es_el_mismo(uint32_t id) {
					return loc_rcv->id_correlacional == id;
				}

				if (list_any_satisfy(get_id_corr, (void*) _es_el_mismo)
						&& pokemon_required(loc_rcv->nombre_pokemon)
						&& pokemon_not_pendant(loc_rcv->nombre_pokemon)
						&& pokemon_in_pokemon_to_catch(loc_rcv->nombre_pokemon)
						&& pokemon_not_localized(loc_rcv->nombre_pokemon)) {
					t_pokemon_received* pokemon = malloc(sizeof(t_pokemon_received));
					pokemon->name = string_new();
					string_append(&pokemon->name, loc_rcv->nombre_pokemon);

					pokemon->pos = list_create();
					pokemon->pos = loc_rcv->posiciones;

					add_to_pokemon_to_catch(pokemon);
					list_add(pokemons_localized, pokemon->name);
				}
				break;
			}

			break;
		}

		case APPEARED_POKEMON: {
			t_appeared_pokemon *appeared_rcv = utils_receive_and_deserialize(fd, protocol);
			team_logger_info("Appeared received! ID correlacional: %d Nombre Pokemon: %s Largo nombre: %d Posición: (%d, %d)",
					appeared_rcv->id_correlacional,
					appeared_rcv->nombre_pokemon, appeared_rcv->tamanio_nombre,
					appeared_rcv->pos_x, appeared_rcv->pos_y);
			usleep(50000);

			if (is_server == 0) {
				t_protocol ack_new = ACK;
				int r = send(fd, &ack_new, sizeof(t_protocol), 0);
				team_logger_warn("%d", r);
				team_logger_warn("ACK sent");
			}

			if (pokemon_required(appeared_rcv->nombre_pokemon)
					&& pokemon_not_pendant(appeared_rcv->nombre_pokemon)
					&& pokemon_in_pokemon_to_catch(appeared_rcv->nombre_pokemon)) {
				t_position* posicion = malloc(sizeof(t_position));
				posicion->pos_x = appeared_rcv->pos_x;
				posicion->pos_y = appeared_rcv->pos_y;
				t_pokemon_received* pokemon = malloc(sizeof(t_pokemon_received));
				pokemon->name = malloc(sizeof(appeared_rcv->tamanio_nombre));
				pokemon->name = appeared_rcv->nombre_pokemon;
				pokemon->pos = list_create();
				list_add(pokemon->pos, posicion);
				add_to_pokemon_to_catch(pokemon);
			}
			break;
		}
		default:
			break;
		}
	}
	return NULL;
}

bool pokemon_not_localized(char* nombre) {
	for (int i = 0; i < list_size(pokemons_localized); i++) {
		char* pokemon = list_get(pokemons_localized, i);
		if (string_equals_ignore_case(pokemon, nombre)) {
			return true;
		}
	}

	return false;
}

void quitar_de_pokemones_pendientes(char* pokemon) {
	for (int i = 0; i < list_size(pokemones_pendientes); i++) {
		char* nombre = list_get(pokemones_pendientes, i);
		if (string_equals_ignore_case(pokemon, nombre)) {
			list_remove(pokemones_pendientes, i);
		}
	}
}

void quitar_de_real_target(char* pokemon) {
	for (int i = 0; i < list_size(real_targets_pokemons); i++) {
		t_pokemon* pok = list_get(real_targets_pokemons, i);
		if (string_equals_ignore_case(pokemon, pok->name)) {
			list_remove(real_targets_pokemons, i);
			break;
		}
	}
}

void add_to_pokemon_to_catch(t_pokemon_received* pokemon) {
	pthread_mutex_lock(&cola_pokemons_a_atrapar);
	list_add(pokemon_to_catch, pokemon);
	pthread_mutex_unlock(&cola_pokemons_a_atrapar);

	sem_post(&sem_message_on_queue);

}

bool trainer_is_in_deadlock_caught(t_entrenador_pokemon* entrenador) {
	list_clean(lista_auxiliar);
	t_list* lista_auxiliar = list_duplicate(entrenador->targets);

	if (list_size(entrenador->pokemons) == list_size(lista_auxiliar) || (list_size(entrenador->pokemons) - list_size(entrenador->targets) == entrenador->diferencia)) {

		for (int i = 0; i < list_size(entrenador->pokemons); i++) {
			t_pokemon* pokemon_obtenido = list_get(entrenador->pokemons, i);

			for (int j = 0; j < list_size(lista_auxiliar); j++) {
				t_pokemon* pokemon_objetivo = list_get(lista_auxiliar, j);
				if (string_equals_ignore_case(pokemon_objetivo->name, pokemon_obtenido->name)) {
					list_remove(lista_auxiliar, j);
				}
			}
		}
		return !list_is_empty(lista_auxiliar);
	}
	list_clean(lista_auxiliar);
	return false;
}

bool pokemon_required(char* pokemon_name) {
	for (int i = 0; i < list_size(real_targets_pokemons); i++) {
		t_pokemon* pokemon = list_get(real_targets_pokemons, i);
		if (string_equals_ignore_case(pokemon_name, pokemon->name)) {
			return true;
		}
	}
	return false;
}

bool pokemon_in_pokemon_to_catch(char* pokemon_name) {
	for (int i = 0; i < list_size(pokemon_to_catch); i++) {
		t_pokemon* pokemon = list_get(pokemon_to_catch, i);
		if (string_equals_ignore_case(pokemon_name, pokemon->name)) {
			return false;
		}
	}
	return true;
}

void team_server_init() {
	team_socket = socket_create_listener(team_config->ip_team, team_config->puerto_team);
	if (team_socket < 0) {
		return;
	}

	while (true) {
		int accepted_fd = socket_accept_conection(team_socket);
		pthread_t tid;

		if (accepted_fd == -1) {
			continue;
		}
		t_handle_connection* connection_handler = malloc(sizeof(t_handle_connection));
		connection_handler->fd = accepted_fd;
		connection_handler->bool_val = 1;

		pthread_create(&tid, NULL, (void*) handle_connection, (void*) connection_handler);
		pthread_detach(tid);
	}
}

void *handle_connection(void *arg) {
	t_handle_connection* connect_handler = (t_handle_connection *) arg;
	int client_fd = connect_handler->fd;
	receive_msg(client_fd, connect_handler->bool_val);
	return NULL;
}

void team_exit() {
	team_planner_print_fullfill_target();
	socket_close_conection(team_socket);
	team_planner_destroy();
	team_config_free();
	team_logger_destroy();
}
