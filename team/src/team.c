#include "team.h"

int main(int argc, char *argv[]) {
	if (team_load() < 0)
		return EXIT_FAILURE;

	team_init();

	team_exit();

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
	sem_init(&sem_pokemons_to_get, 0, 1);
	sem_init(&sem_message_on_queue, 0, 0);
	sem_init(&sem_planificador, 0, 1);
	sem_init(&sem_trainers_in_ready_queue, 0, 0);
	sem_init(&sem_deadlock, 0, 0);
	pthread_mutex_init(&cola_pokemons_a_atrapar, NULL);
	message_catch_sended = list_create();
	pokemones_pendientes = list_create();
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
	sem_wait(&sem_entrenadores_disponibles);  

	team_logger_info("Creando un hilo para subscribirse a la cola APPEARED del broker %d");
	t_cola cola_appeared = APPEARED_QUEUE;
	pthread_create(&tid1, NULL, (void*) team_retry_connect, (void*) &cola_appeared);
	pthread_detach(tid1);

	team_logger_info("Creando un hilo para subscribirse a la cola LOCALIZED del broker %d");

	t_cola cola_localized = LOCALIZED_QUEUE;
	pthread_create(&tid2, NULL, (void*) team_retry_connect, (void*) &cola_localized);
	pthread_detach(tid2);

	team_logger_info("Creando un hilo para subscribirse a la cola CAUGHT del broker %d");

	t_cola cola_caught = CAUGHT_QUEUE;
	pthread_create(&tid3, NULL, (void*) team_retry_connect, (void*) &cola_caught);
	pthread_detach(tid3);

	pthread_create(&algoritmo_cercania_entrenadores, NULL, (void*) team_planner_algoritmo_cercania, NULL);
	team_logger_info("Creando un hilo que maneje el ALGORITMO DE CERCANÍA");
	pthread_detach(algoritmo_cercania_entrenadores);

	pthread_create(&planificator, NULL, (void*) team_planner_run_planification, NULL);
	team_logger_info("Creando un hilo que maneje la PLANIFICACIÓN");
	pthread_detach(planificator);

	team_logger_info("Creando un hilo para poner al Team en modo Servidor");
	team_server_init();
	usleep(500000);
}


void remove_pokemon_from_catch (t_pokemon* pokemon) {
	for (int i = 0; i < list_size(pokemon_to_catch); i++) {
		t_pokemon_received* pokemon_con_posiciones = list_get(pokemon_to_catch, i);

		if (string_equals_ignore_case(pokemon_con_posiciones->name, pokemon->name)) {
			t_list* posiciones_pokemon = list_create();
			posiciones_pokemon = pokemon_con_posiciones->pos;

			if(list_size(posiciones_pokemon) == 1){
				pthread_mutex_lock(&cola_pokemons_a_atrapar);
				list_remove(pokemon_to_catch, i);
				pthread_mutex_unlock(&cola_pokemons_a_atrapar);
			} else {

				for (int j = 0; j < list_size(posiciones_pokemon); j++) {
					t_position* position = list_get(posiciones_pokemon, j);
					if (position->pos_x == pokemon->position->pos_x && position->pos_y ==  pokemon->position->pos_x) {
						pthread_mutex_lock(&cola_pokemons_a_atrapar);
						list_remove(pokemon_con_posiciones->pos, j);
						pthread_mutex_unlock(&cola_pokemons_a_atrapar);
					}
				}
			}
		}
	}
}


void send_message_catch(t_catch_pokemon* catch_send, t_entrenador_pokemon* entrenador) {
	t_protocol catch_protocol = CATCH_POKEMON;
	

	entrenador->state = BLOCK;
	list_add(block_queue, entrenador);
	team_logger_info("Se añadió al entrenador %d a la cola de BLOCK luego de intentar un CATCH a la espera de confirmación", entrenador->id);

	int i = send_message(catch_send, catch_protocol, NULL);
	
	if (i == 0) {
		team_logger_info("Mensaje CATCH enviado! Pokemon: %s, posición (%d, %d)", catch_send->nombre_pokemon, catch_send->pos_x, catch_send->pos_y);
		team_planner_change_block_status_by_id_corr(1, catch_send->id_correlacional);
		list_add(message_catch_sended, catch_send);
		list_add(entrenador->list_id_catch, (void*)catch_send->id_correlacional);
	} else {
		team_logger_warn("No se ha podido enviar el mensaje CATCH. Se agregará a los pokemons atrapados del entrenador %d, por comportamiento default: %s, posición (%d, %d)", entrenador->id, catch_send->nombre_pokemon, catch_send->pos_x, catch_send->pos_y);
		team_planner_change_block_status_by_trainer(0, entrenador);
		t_pokemon* pokemon = team_planner_pokemon_create(catch_send->nombre_pokemon);
		list_add(entrenador->pokemons, pokemon);
		team_logger_info("El entrenador %d atrapó un %s!!", entrenador->id, catch_send->nombre_pokemon);
		quitar_de_pokemones_pendientes(entrenador->pokemon_a_atrapar->name);

		if (trainer_is_in_deadlock_caught(entrenador)) {
			team_logger_info("El entrenador %d está en deadlock!", entrenador->id);
			entrenador->deadlock = true;
		}
		
		if (trainer_completed_with_success(entrenador)) {
			team_planner_finish_trainner(entrenador);
		}

		if (all_queues_are_empty_except_block()) {//TODO: esta funcion debería devolver true si lo unico que tiene elementos es blocked en deadlock, pero no lo hace
			pthread_mutex_lock(&entrenador->sem_move_trainers);
			team_logger_info("Ya no es posible atrapar más pokemones debido a que se alcanzó la cantidad del objetivo!");
			solve_deadlock();
		}		
	}
	usleep(500000);
}


bool pokemon_not_pendant(char* pokemon){
	for(int i = 0; i < list_size(pokemones_pendientes); i++){
		char* nombre = list_get(pokemones_pendientes, i);

		if(string_equals_ignore_case(nombre, pokemon)){
			return false;
		}
	}
	return true;
}


void send_get_message() {	
	sem_wait(&sem_pokemons_to_get);
	t_protocol get_protocol;
	t_get_pokemon* get_send = malloc(sizeof(t_get_pokemon));

	for (int i = 0; i < list_size(keys_list); i++) {
		char* nombre = list_get(keys_list, i);
		get_send->id_correlacional = 0;
		get_send->nombre_pokemon = string_duplicate(nombre);
		get_send->tamanio_nombre = strlen(get_send->nombre_pokemon) + 1;
		get_protocol = GET_POKEMON;
		
		int i = send_message(get_send, get_protocol, get_id_corr);
		if (i==0){
			team_logger_info("Se envió un mensaje GET del Pokemon %s", get_send->nombre_pokemon);
		}
		if (i > 0) {
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
			list_add(queue, (void*)id_corr);
		}
		if (protocolo == CATCH_POKEMON) {
			t_catch_pokemon *catch_send = (t_catch_pokemon*) paquete;
			catch_send->id_correlacional = id_corr;
		}
		socket_close_conection(broker_fd_send);//cambie broker_fd por broker_fd_send.
	}
	return 0;
}


void check_RR_burst(t_entrenador_pokemon* entrenador) {
	if(entrenador->current_burst_time == team_config->quantum) {
		pthread_mutex_lock(&entrenador->sem_move_trainers);
		sem_post(&sem_planificador);
		add_to_ready_queue(entrenador);	
		team_logger_info("Se añadió al entrenador %d a la cola de READY ya que terminó su QUANTUM", entrenador->id);
	}
}


void check_SJF_CD_time(t_entrenador_pokemon* entrenador) {

	if (entrenador->estimated_time > team_planner_get_least_estimate_index()) {
		pthread_mutex_lock(&entrenador->sem_move_trainers);
		sem_post(&sem_planificador);
		add_to_ready_queue(entrenador);	
		team_logger_info("Se añadió al entrenador %d a la cola de READY ya que será desalojado por otro con ráfaga más corta", entrenador->id);
	}
}


void move_trainers_and_catch_pokemon(t_entrenador_pokemon* entrenador) {
	while(true){
		pthread_mutex_lock(&entrenador->sem_move_trainers);

		int aux_x = entrenador->position->pos_x - entrenador->pokemon_a_atrapar->position->pos_x;
		int	aux_y = entrenador->position->pos_y - entrenador->pokemon_a_atrapar->position->pos_y;

		int steps = fabs(aux_x + aux_y);

		for (int i = 0; i <= steps; i++) {
			sleep(team_config->retardo_ciclo_cpu);
			new_cpu_cicle();
			entrenador->current_burst_time++;
			entrenador->total_burst_time++; 

			if (team_config->algoritmo_planificacion == RR) {
				check_RR_burst(entrenador); 
			} 

			if (team_config->algoritmo_planificacion == SJF_CD) {
				check_SJF_CD_time(entrenador); 
			}
		}
		
		team_logger_info("El entrenador %d se movió de (%d, %d) a (%d, %d)", entrenador->id,
																			entrenador->position->pos_x,
																			entrenador->position->pos_y,
																			entrenador->pokemon_a_atrapar->position->pos_x,
																			entrenador->pokemon_a_atrapar->position->pos_y);

		entrenador->position->pos_x = entrenador->pokemon_a_atrapar->position->pos_x;
		entrenador->position->pos_y = entrenador->pokemon_a_atrapar->position->pos_x;

		if (entrenador->deadlock) {		
			sem_post(&sem_deadlock);
		}

		if (entrenador->blocked_info == 0 && entrenador->deadlock == false) {
			t_catch_pokemon* catch_send = malloc(sizeof(t_catch_pokemon));
			catch_send->id_correlacional = 0;
			catch_send->nombre_pokemon = entrenador->pokemon_a_atrapar->name;
			catch_send->pos_x = entrenador->pokemon_a_atrapar->position->pos_x;
			catch_send->pos_y = entrenador->pokemon_a_atrapar->position->pos_y;
			catch_send->tamanio_nombre = strlen(catch_send->nombre_pokemon);
			send_message_catch(catch_send, entrenador);
			sem_post(&sem_planificador);
		}
		
		pthread_mutex_lock(&entrenador->sem_move_trainers);
	}
}

void subscribe_to(void *arg) {

	t_cola cola = *((int *) arg);
	team_logger_info("tipo Cola: %d ", cola);
	switch (cola) {
		case NEW_QUEUE: {
			team_logger_info("Cola NEW ");
			break;
		}
		case CATCH_QUEUE: {
			team_logger_info("Cola CATCH ");
			break;
		}
		case CAUGHT_QUEUE: {
			team_logger_info("Cola CAUGHT ");
			break;
		}
		case GET_QUEUE: {
			team_logger_info("Cola GET ");
			break;
		}
		case LOCALIZED_QUEUE: {
			team_logger_info("Cola LOCALIZED ");
			break;
		}
		case APPEARED_QUEUE: {
			team_logger_info("Cola APPEARED ");
			break;
		}
	}

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
				team_logger_info("Caught received");
				t_caught_pokemon *caught_rcv = utils_receive_and_deserialize(fd, protocol);
				team_logger_info("ID correlacional: %d", caught_rcv->id_correlacional);
				team_logger_info("Resultado (0/1): %d", caught_rcv->result);

				t_catch_pokemon* catch_message = filter_msg_catch_by_id_caught(caught_rcv->id_correlacional);				
				t_entrenador_pokemon* entrenador = filter_trainer_by_id_caught(caught_rcv->id_correlacional);

				team_planner_change_block_status_by_id_corr(0, caught_rcv->id_correlacional);
				quitar_de_pokemones_pendientes(catch_message->nombre_pokemon);

				if(caught_rcv->result) {
					team_logger_info("MENSAJE CAUGHT POSITIVO: El entrenador %d, atrapó un %s!!", entrenador->id, catch_message->nombre_pokemon);
					list_add(entrenador->pokemons, catch_message->nombre_pokemon);


					if(trainer_completed_with_success(entrenador)){
						team_planner_finish_trainner(entrenador);
					}

					if(trainer_is_in_deadlock_caught(entrenador)){
						entrenador->deadlock = true;
					} 

					if (all_queues_are_empty_except_block()) {
						team_logger_info("Ya no es posible atrapar más pokemones debido a que se alcanzó la cantidad del objetivo!");
						solve_deadlock();
					}

				} else {
					team_logger_info("MENSAJE CAUGHT NEGATIVO: El entrenador %d no atrapó a %s, al no tener mensajes pendientes volverá a poder ser planificado.", entrenador->id, catch_message->nombre_pokemon);
				}

				usleep(50000);
				break;
			}

			case LOCALIZED_POKEMON: {
				team_logger_info("Localized received!");
				t_localized_pokemon *loc_rcv = utils_receive_and_deserialize(fd, protocol);
				team_logger_info("ID correlacional: %d", loc_rcv->id_correlacional);
				team_logger_info("Nombre Pokemon: %s", loc_rcv->nombre_pokemon);
				team_logger_info("Largo nombre: %d", loc_rcv->tamanio_nombre);
				team_logger_info("Cant elementos en lista: %d", loc_rcv->cant_elem); 
				for (int el = 0; el < loc_rcv->cant_elem; el++) {
					t_position* pos = malloc(sizeof(t_position));
					pos = list_get(loc_rcv->posiciones, el);
					team_logger_info("Posición: (%d, %d)", pos->pos_x, pos->pos_y);
				}
				usleep(500000);

				bool _es_el_mismo(uint32_t id) {
					return loc_rcv->id_correlacional == id;
				}

				if (list_any_satisfy(get_id_corr, (void*) _es_el_mismo) && pokemon_required(loc_rcv->nombre_pokemon) && pokemon_not_pendant(appeared_rcv->nombre_pokemon)){
					t_pokemon_received* pokemon = malloc(sizeof(t_pokemon_received));
					pokemon->name = malloc(sizeof(loc_rcv->tamanio_nombre));
					pokemon->name = loc_rcv->nombre_pokemon;
					pokemon->pos = list_create();
					pokemon->pos = loc_rcv->posiciones;
					add_to_pokemon_to_catch(pokemon);
				}
				break;
			}

			case APPEARED_POKEMON: {
				team_logger_info("Appeared received!");
				t_appeared_pokemon *appeared_rcv = utils_receive_and_deserialize(fd, protocol);
				team_logger_info("ID correlacional: %d", appeared_rcv->id_correlacional);
				team_logger_info("Nombre Pokemon: %s", appeared_rcv->nombre_pokemon);
				team_logger_info("Largo nombre: %d", appeared_rcv->tamanio_nombre);
				team_logger_info("Posición: (%d, %d)", appeared_rcv->pos_x, appeared_rcv->pos_y);
				usleep(50000);

				if (is_server == 0) {
					pthread_t tid;
					pthread_create(&tid, NULL, (void*) send_ack, (void*) &appeared_rcv->id_correlacional);
					pthread_detach(tid);
				}

				if (pokemon_required(appeared_rcv->nombre_pokemon) && pokemon_not_pendant(appeared_rcv->nombre_pokemon)) {
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


void quitar_de_pokemones_pendientes(char* pokemon){
	for(int i = 0, i < list_size(pokemones_pendientes); i++){
		char* nombre = list_get(pokemones_pendientes, i);
		if(string_equals_ignore_case(pokemon, nombre)){
			lost_remove(pokemones_pendientes, i);
		}
	}
}

void add_to_pokemon_to_catch(t_pokemon_received* pokemon) {
	pthread_mutex_lock(&cola_pokemons_a_atrapar);
	list_add(pokemon_to_catch, pokemon);
	pthread_mutex_unlock(&cola_pokemons_a_atrapar);

	for(int i = 0; i < list_size(pokemon->pos); i++){
		sem_post(&sem_message_on_queue);
	}
	team_logger_info("Se añadió a %s a la cola de pokemons a atrapar.", pokemon->name);
}

bool trainer_is_in_deadlock_caught(t_entrenador_pokemon* entrenador) {
	t_list* targets_aux = entrenador->targets;
	if (list_size(entrenador->pokemons) == list_size(entrenador->targets)) {

		for (int i = 0; i < list_size(entrenador->pokemons); i++) {
			char* pokemon_obtenido = list_get(entrenador->pokemons, i);

			for (int j = 0; j < list_size(targets_aux); j++) {
				char* pokemon_objetivo = list_get(targets_aux, j);
				if (string_equals_ignore_case(pokemon_objetivo, pokemon_obtenido)) {
					list_remove(target_pokemons, j);
				}
			}
		}
	}
	return list_size(targets_aux) == 0;
}

bool pokemon_required(char* pokemon_name) {

	bool _es_el_mismo(char* name) {
		return  string_equals_ignore_case(pokemon_name,name);
	}

	char* _get_name(t_pokemon_received* pokemon) {
		return pokemon->name;
	}

	t_list* pokemon_to_catch_name = list_map(pokemon_to_catch, (void*) _get_name);

	return !list_any_satisfy(pokemon_to_catch_name, (void*) _es_el_mismo);
}

void team_server_init() {

	team_socket = socket_create_listener(team_config->ip_team, team_config->puerto_team);
	if (team_socket < 0) {
		team_logger_error("Error al crear server.");
		return;
	}

	team_logger_info("Server creado correctamente!! Esperando conexiones...");

	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;

	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

	for (;;) {
		int accepted_fd;
		pthread_t tid;

		if ((accepted_fd = accept(team_socket, (struct sockaddr *) &client_info, &addrlen)) != -1) {

			t_handle_connection* connection_handler = malloc( sizeof(t_handle_connection));
			connection_handler->fd = accepted_fd;
			connection_handler->bool_val = 1;

			pthread_create(&tid, NULL, (void*) handle_connection, (void*) connection_handler);
			pthread_detach(tid);
			team_logger_info("Creando un hilo para atender una conexión en el socket %d", accepted_fd);
		} else {
			team_logger_error("Error al conectar con un cliente");
		}
	}
}

void *handle_connection(void *arg) {
	t_handle_connection* connect_handler = (t_handle_connection *) arg;
	int client_fd = connect_handler->fd;
	receive_msg(client_fd, connect_handler->bool_val);
	return NULL;
}


void send_ack(void* arg) {
	int val = *((int*) arg);
	t_ack* ack_snd = malloc(sizeof(t_ack));
	t_protocol ack_protocol = ACK;
	ack_snd->id = val;

	int client_fd = socket_connect_to_server(team_config->ip_broker, team_config->puerto_broker);
	if (client_fd > 0) {
		utils_serialize_and_send(client_fd, ack_protocol, ack_snd);
		team_logger_info("ACK SENT TO BROKER");
	}
	team_logger_info("CONNECTION WITH BROKER WILL BE CLOSED");
	socket_close_conection(client_fd);
}


void team_exit() {
	team_planner_print_fullfill_target();
	socket_close_conection(team_socket);
	team_planner_destroy();
	team_config_free();
	team_logger_destroy();
}
