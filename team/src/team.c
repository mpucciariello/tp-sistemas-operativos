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

	sem_init(&sem_entrenadores_disponibles, 0, 0);
	sem_init(&sem_pokemons_to_get, 0, 0);
	sem_init(&sem_message_on_queue, 0, 0);
	sem_init(&sem_planificador, 0, 1);
	sem_init(&sem_trainers_in_ready_queue, 0, 0);
	sem_init(&appeared_recibido, 0, 0);

	pthread_mutex_init(&cola_pokemons_a_atrapar, NULL);

	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
	pthread_t tid1;
	pthread_t tid2;
	pthread_t tid3;
	pthread_t planificator;
	pthread_t msg_get;
	pthread_t algoritmo_cercania_entrenadores;

	appeared_recibidos = false;

	team_planner_init();

	t_cola cola_appeared = APPEARED_QUEUE;
	pthread_create(&tid1, NULL, (void*) team_retry_connect_1, (void*) &cola_appeared);
	pthread_detach(tid1);

	already_printed = true;
	t_cola cola_localized = LOCALIZED_QUEUE;
	pthread_create(&tid2, NULL, (void*) team_retry_connect, (void*) &cola_localized);
	pthread_detach(tid2);

	t_cola cola_caught = CAUGHT_QUEUE;
	pthread_create(&tid3, NULL, (void*) team_retry_connect,	(void*) &cola_caught);
	pthread_detach(tid3);

	pthread_create(&algoritmo_cercania_entrenadores, NULL,(void*) team_planner_algoritmo_cercania, NULL);
	pthread_detach(algoritmo_cercania_entrenadores);

	pthread_create(&planificator, NULL, (void*) team_planner_run_planification, NULL);
	pthread_detach(planificator);

	pthread_create(&msg_get, NULL, (void*) send_get_message, NULL);
	pthread_detach(msg_get);

	team_server_init();
	usleep(500000);

}

void team_planner_remove_pokemon_from_catch(t_pokemon* pokemon) {

	list_clean(lista_auxiliar);
	for (int i = 0; i < list_size(pokemon_to_catch); i++) {
		t_pokemon_received* pokemon_con_posiciones = list_get(pokemon_to_catch,	i);

		if (string_equals_ignore_case(pokemon_con_posiciones->name, pokemon->name)) {
			lista_auxiliar = pokemon_con_posiciones->pos;

			if (list_size(lista_auxiliar) == 1) {
				pthread_mutex_lock(&cola_pokemons_a_atrapar);
				list_remove(pokemon_to_catch, i);
				pthread_mutex_unlock(&cola_pokemons_a_atrapar);
			} else {

				for (int j = 0; j < list_size(lista_auxiliar); j++) {
					t_position* position = list_get(lista_auxiliar, j);
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
	list_clean(lista_auxiliar);
}

void send_message_catch(t_catch_pokemon* catch_send,t_entrenador_pokemon* entrenador) {
	t_protocol catch_protocol = CATCH_POKEMON;
	entrenador->state = BLOCK;

	list_add(block_queue, entrenador);
	team_logger_info("El entrenador %d pasó a la cola de BLOCK luego de enviar un mensaje CATCH.", entrenador->id);
	team_planner_change_block_status_by_id_corr(false, catch_send->id_correlacional);

	int i = send_message(catch_send, catch_protocol, NULL);
	if (i == 0) {
		list_add(message_catch_sended, catch_send);
		list_add(entrenador->list_id_catch, (void*) catch_send->id_correlacional);
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
			pthread_mutex_lock(&cola_pokemons_a_atrapar);
			list_remove(pokemon_to_catch, i);
			pthread_mutex_unlock(&cola_pokemons_a_atrapar);
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
	t_pokemon* pokemon = team_planner_pokemon_create(pokemon_name);
	list_add(entrenador->pokemons, pokemon);
	team_logger_info("El entrenador %d atrapó un %s en la posición (%d, %d)!!",	entrenador->id, pokemon_name, entrenador->pokemon_a_atrapar->position->pos_x, entrenador->pokemon_a_atrapar->position->pos_y);
	quitar_de_pokemones_pendientes(pokemon_name);
	quitar_de_real_target(pokemon_name);
	team_planner_change_block_status_by_trainer(true, entrenador);


	if (team_planner_trainer_completed_with_success(entrenador)) {
		team_planner_finish_trainner(entrenador);
	}

	if (trainer_is_in_deadlock_caught(entrenador)) {
		entrenador->deadlock = true;
	}


	if (team_planner_all_queues_are_empty_except_block()) {
		team_planner_solve_deadlock();
	}

	if (team_planner_all_finished()) {
		team_logger_info("El team se encuentra en condiciones de FINALIZAR!");
		team_planner_print_fullfill_target();
		team_planner_exit();
		socket_close_conection(team_socket);
		exit(0);
	}

	if (todavia_quedan_pokemones_restantes(pokemon_name)) {
		if (tengo_en_pokemon_to_catch(pokemon_name)) {
			sem_post(&sem_message_on_queue);
		}
	} else {
		if (tengo_en_pokemon_to_catch(pokemon_name)) {
			remover_totalmente_de_pokemon_to_catch(pokemon_name);
		}
	}

	if (entrenador->status && !entrenador->deadlock && entrenador->state == BLOCK && !trainer_is_in_deadlock_caught(entrenador) && !team_planner_trainer_completed_with_success(entrenador)) {
		sem_post(&sem_entrenadores_disponibles);
	}
}

bool team_planner_all_finished() {
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
	sem_wait(&appeared_recibido);
	sleep(5);
	t_protocol get_protocol;
	t_get_pokemon* get_send = malloc(sizeof(t_get_pokemon));

	for (int i = 0; i < list_size(real_targets_pokemons); i++) {
		t_pokemon* pk = list_get(real_targets_pokemons, i);
		get_send->id_correlacional = 0;
		get_send->nombre_pokemon = string_duplicate(pk->name);
		get_send->tamanio_nombre = strlen(get_send->nombre_pokemon) + 1;

		get_protocol = GET_POKEMON;
		if ((send_message(get_send, get_protocol, get_id_corr)) > 0) {
			team_logger_info("Se recibió ID CORR %d en respuesta mensaje GET.", get_id_corr);
		}
		usleep(500000);
	}
}

int send_message(void* paquete, t_protocol protocolo, t_list* queue) {
	int broker_fd_send = socket_connect_to_server(team_config->ip_broker, team_config->puerto_broker);

	if (broker_fd_send < 0) {
		socket_close_conection(broker_fd_send);
		return -1;
	} else {
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

void team_planner_check_RR_burst(t_entrenador_pokemon* entrenador) {
	if (entrenador->current_burst_time == team_config->quantum) {
		team_planner_add_to_ready_queue(entrenador);
		sem_post(&sem_trainers_in_ready_queue);
		sem_post(&sem_planificador);
		team_logger_info("El entrenador %d paso a la cola de READY ya que terminó su QUANTUM.", entrenador->id);
		pthread_mutex_lock(&entrenador->sem_move_trainers);
	}
}

void team_planner_check_SJF_CD_time(t_entrenador_pokemon* entrenador) {
	t_entrenador_pokemon* lower_estimated_trainer = list_get(ready_queue, team_planner_get_least_estimate_index());
	if (lower_estimated_trainer != NULL) {
		if (entrenador->estimated_time > lower_estimated_trainer->estimated_time) {
			team_planner_add_to_ready_queue(entrenador);
			sem_post(&sem_trainers_in_ready_queue);
			sem_post(&sem_planificador);
			team_logger_info("El entrenador %d pasó a la cola de READY ya que será desalojado.", entrenador->id);
			pthread_mutex_lock(&entrenador->sem_move_trainers);
		}
	}
}

void move_trainers_and_catch_pokemon(t_entrenador_pokemon* entrenador) {
	while (true) {
		pthread_mutex_lock(&entrenador->sem_move_trainers);
		int aux_x = entrenador->position->pos_x - entrenador->pokemon_a_atrapar->position->pos_x;
		int aux_y = entrenador->position->pos_y - entrenador->pokemon_a_atrapar->position->pos_y;

		int steps = fabs(aux_x) + fabs(aux_y);

		if (entrenador->position->pos_x != entrenador->pokemon_a_atrapar->position->pos_x || entrenador->position->pos_y != entrenador->pokemon_a_atrapar->position->pos_y) {
			for (int i = 0; i < steps; i++) {
				sleep(team_config->retardo_ciclo_cpu);
				team_planner_new_cpu_cicle(entrenador);
			}
		}

		if (!entrenador->se_movio) {
			team_logger_info("El entrenador %d se movió de (%d, %d) a (%d, %d).", entrenador->id,
																				  entrenador->position->pos_x,
																				  entrenador->position->pos_y,
																				  entrenador->pokemon_a_atrapar->position->pos_x,
																				  entrenador->pokemon_a_atrapar->position->pos_y);

			entrenador->position->pos_x = entrenador->pokemon_a_atrapar->position->pos_x;
			entrenador->position->pos_y = entrenador->pokemon_a_atrapar->position->pos_y;

			team_planner_new_cpu_cicle(entrenador);
			entrenador->se_movio = true;
		}

		t_catch_pokemon* catch_send = malloc(sizeof(t_catch_pokemon));
		catch_send->id_correlacional = 0;
		catch_send->nombre_pokemon = string_duplicate(entrenador->pokemon_a_atrapar->name);
		catch_send->tamanio_nombre = strlen(catch_send->nombre_pokemon);
		catch_send->pos_x = entrenador->pokemon_a_atrapar->position->pos_x;
		catch_send->pos_y = entrenador->pokemon_a_atrapar->position->pos_y;

		send_message_catch(catch_send, entrenador);

		entrenador->se_movio = false;
		sem_post(&sem_planificador);
	}
}

void subscribe_to(void *arg) {
	t_cola cola = *((int *) arg);
	int new_broker_fd = socket_connect_to_server(team_config->ip_broker, team_config->puerto_broker);

	if (new_broker_fd < 0) {
		socket_close_conection(new_broker_fd);
	} else {
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

void subscribe_to1(void *arg) {
	t_cola cola = *((int *) arg);
	int new_broker_fd = socket_connect_to_server(team_config->ip_broker, team_config->puerto_broker);

	if (new_broker_fd < 0) {
		team_logger_warn("No se pudo conectar con BROKER porque no se encuentra activo. Se realizará la operación por DEFAULT.");
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

void team_retry_connect_1(void* arg) {
	void* arg2 = arg;

	team_logger_info("Inicio de proceso de reintento de comunicación con el BROKER.");

	while (true) {
		is_connected = false;
		subscribe_to1(arg2);
		utils_delay(team_config->tiempo_reconexion);

		if (!is_connected) {
			team_logger_warn("El reintento de comunicación con el BROKER NO fue exitoso.");
		} else {
			team_logger_info("El reintento de comunicación con el BROKER fue EXITOSO.");
		}
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
			return NULL;
		}

		switch (protocol) {

		case CAUGHT_POKEMON: {
			t_caught_pokemon *caught_rcv = utils_receive_and_deserialize(fd, protocol);
			team_logger_info("Se recibió un ID CORRELACIONAL de un mensaje CAUGHT: %d. Resultado (0/1): %d", caught_rcv->id_correlacional, caught_rcv->result);

			if (is_server == 0) {
				t_protocol ack_protocol = ACK;
				team_logger_info("ACK SENT TO BROKER");

				t_ack* ack_send = malloc(sizeof(t_ack));
				ack_send->id_corr_msg = caught_rcv->id_correlacional;
				ack_send->queue = CAUGHT_QUEUE;
				ack_send->sender_name = "TEAM";
				ack_send->ip = team_config->ip_team;
				ack_send->port = team_config->puerto_team;

				utils_serialize_and_send(fd, ack_protocol, ack_send);
				usleep(500000);
			}

			t_catch_pokemon* catch_message = filter_msg_catch_by_id_caught(caught_rcv->id_correlacional);
			t_entrenador_pokemon* entrenador = filter_trainer_by_id_caught(caught_rcv->id_correlacional);

			quitar_de_pokemones_pendientes(catch_message->nombre_pokemon);

			if (caught_rcv->result == 1) {
				atrapar_pokemon(entrenador, catch_message->nombre_pokemon);
			} else {
				team_planner_change_block_status_by_trainer(true, entrenador);
				team_logger_info("El entrenador %d NO pudo atrapar un %s en la posición (%d, %d).", entrenador->id, catch_message->nombre_pokemon, catch_message->pos_x, catch_message->pos_y);
			}
			break;
		}

		case LOCALIZED_POKEMON: {
			t_localized_pokemon *loc_rcv = utils_receive_and_deserialize(fd, protocol);
			team_logger_info("Se recibió un LOCALIZED! ID: %d. Nombre Pokemon: %s. Largo Nombre: %d. Cant elementos en lista: %d.",
					loc_rcv->id_correlacional, loc_rcv->nombre_pokemon, loc_rcv->tamanio_nombre, loc_rcv->cant_elem);

			if (loc_rcv->cant_elem > 0) {
				if (is_server == 0) {
					t_protocol ack_protocol = ACK;
					team_logger_info("ACK SENT TO BROKER");

					t_ack* ack_send = malloc(sizeof(t_ack));
					ack_send->id_corr_msg = loc_rcv->id_correlacional;
					ack_send->queue = LOCALIZED_QUEUE;
					ack_send->sender_name = "TEAM";
					ack_send->ip = team_config->ip_team;
					ack_send->port = team_config->puerto_team;

					utils_serialize_and_send(fd, ack_protocol, ack_send);
					usleep(500000);
				}

				bool _es_el_mismo(uint32_t id) {
					return loc_rcv->id_correlacional == id;
				}

				if (list_any_satisfy(get_id_corr, (void*) _es_el_mismo)
						&& pokemon_required(loc_rcv->nombre_pokemon)
						&& pokemon_not_pendant(loc_rcv->nombre_pokemon)) {
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
			team_logger_info("Se recibió un APPEARED! ID: %d. Nombre Pokemon: %s. Largo Nombre: %d. Posición: (%d, %d).",
					appeared_rcv->id_correlacional,
					appeared_rcv->nombre_pokemon, appeared_rcv->tamanio_nombre,
					appeared_rcv->pos_x, appeared_rcv->pos_y);

			if (is_server == 0) {
				t_protocol ack_protocol = ACK;
				team_logger_info("Se envió ACK.");

				t_ack* ack_send = malloc(sizeof(t_ack));
				ack_send->id_corr_msg = appeared_rcv->id_correlacional;
				ack_send->queue = APPEARED_QUEUE;
				ack_send->sender_name = "TEAM";
				ack_send->ip = team_config->ip_team;
				ack_send->port = team_config->puerto_team;

				utils_serialize_and_send(fd, ack_protocol, ack_send);
				usleep(500000);
			}

			if (pokemon_required(appeared_rcv->nombre_pokemon)) {
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

			if (!appeared_recibidos) {
				appeared_recibidos = true;
				sem_post(&appeared_recibido);
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
			return false;
		}
	}
	return true;
}

void quitar_de_pokemones_pendientes(char* pokemon) {
	if (!list_is_empty(pokemones_pendientes)) {
		for (int i = 0; i < list_size(pokemones_pendientes); i++) {
			char* nombre = list_get(pokemones_pendientes, i);
			if (string_equals_ignore_case(pokemon, nombre)) {
				list_remove(pokemones_pendientes, i);
				break;
			}
		}
	}
}

void quitar_de_real_target(char* pokemon) {
	if (!list_is_empty(real_targets_pokemons)) {
		for (int i = 0; i < list_size(real_targets_pokemons); i++) {
			t_pokemon* pok = list_get(real_targets_pokemons, i);
			if (string_equals_ignore_case(pokemon, pok->name)) {
				list_remove(real_targets_pokemons, i);
				break;
			}
		}
	}
}

void add_to_pokemon_to_catch(t_pokemon_received* pokemon) {
	pthread_mutex_lock(&cola_pokemons_a_atrapar);
	list_add(pokemon_to_catch, pokemon);
	pthread_mutex_unlock(&cola_pokemons_a_atrapar);

	if (pokemon_not_pendant(pokemon->name)) {
		sem_post(&sem_message_on_queue);
	}
}

bool trainer_is_in_deadlock_caught(t_entrenador_pokemon* entrenador) {
	list_clean(lista_auxiliar);
	t_list* lista_auxiliar = list_duplicate(entrenador->targets);

	if (list_size(entrenador->pokemons) == list_size(lista_auxiliar) || ((list_size(entrenador->pokemons) - list_size(entrenador->targets) == entrenador->diferencia))) {

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
		t_pokemon_received* pokemon = list_get(pokemon_to_catch, i);
		if (string_equals_ignore_case(pokemon_name, pokemon->name)) {
			return true;
		}
	}
	return false;
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

