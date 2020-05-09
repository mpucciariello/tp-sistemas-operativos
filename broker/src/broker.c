#include "broker.h"

int main(int argc, char *argv[]) {
	initialize_queue();
	if (broker_load() < 0)
		return EXIT_FAILURE;
	memory = malloc(broker_config->tamano_memoria);
	pointer = 0;
	broker_server_init();
	broker_exit();

	return EXIT_SUCCESS;
}

int broker_load() {
	int response = broker_config_load();
	if (response < 0)
		return response;

	response = broker_logger_create(broker_config->log_file);
	if (response < 0) {
		broker_config_free();
		return response;
	}
	broker_print_config();
    //create mutex for pointer
	if (pthread_mutex_init(&mpointer, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

	return 0;
}

void broker_server_init() {

	broker_socket = socket_create_listener(broker_config->ip_broker,
			broker_config->puerto_broker);
	if (broker_socket < 0) {
		broker_logger_error("Error al crear server");
		return;
	}

	broker_logger_info("Server creado correctamente!! Esperando conexiones...");

	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;

	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

	int accepted_fd;
	for (;;) {

		pthread_t tid;
		if ((accepted_fd = accept(broker_socket,
				(struct sockaddr *) &client_info, &addrlen)) != -1) {

			broker_logger_info(
					"Creando un hilo para atender una conexión en el socket %d",
					accepted_fd);
			pthread_create(&tid, NULL, (void*) handle_connection,
					(void*) &accepted_fd);
			pthread_detach(tid);
			usleep(500000);
		} else {
			broker_logger_error("Error al conectar con un cliente");
		}
	}
}
static void *handle_connection(void *arg) {
	int client_fd = *((int *) arg);
	uint32_t uuid = 0000;

	t_protocol new_protocol;
	t_protocol catch_protocol;
	t_protocol caught_protocol;
	t_protocol get_protocol;
	t_protocol appeared_protocol;
	t_protocol localized_protocol;

	broker_logger_info("Conexion establecida con cliente: %d", client_fd);

	int received_bytes;
	int protocol;
	while (true) {
		received_bytes = recv(client_fd, &protocol, sizeof(int), 0);

		if (received_bytes <= 0) {
			broker_logger_error("Se perdio la conexion");
			return NULL;
		}
		switch (protocol) {

		// From Team o GC
		case ACK: {

			broker_logger_info("ACK RECEIVED");
			t_ack *ack_receive = utils_receive_and_deserialize(client_fd,
					protocol);
			broker_logger_info("ID recibido: %d", ack_receive->id);
			broker_logger_info("ID correlacional %d",
					ack_receive->id_correlacional);
			usleep(100000);
			break;
		}

			// From GB
		case NEW_POKEMON: {
			broker_logger_info("NEW RECEIVED FROM GB");
			t_new_pokemon *new_receive = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID recibido: %d", new_receive->id);
			broker_logger_info("ID Correlacional: %d",
					new_receive->id_correlacional);
			broker_logger_info("Cantidad: %d", new_receive->cantidad);
			broker_logger_info("Nombre Pokemon: %s",
					new_receive->nombre_pokemon);
			broker_logger_info("Largo Nombre: %d", new_receive->tamanio_nombre);
			broker_logger_info("Posicion X: %d", new_receive->pos_x);
			broker_logger_info("Posicion Y: %d", new_receive->pos_y);
			usleep(100000);
			t_message_to_void *message_void = convert_to_void(protocol, new_receive);
			int from = save_on_memory(message_void);
			get_from_memory(protocol, from, memory);
			//free(message_void->message);
			//free(message_void);
			// To GC
			t_new_pokemon* new_snd = malloc(sizeof(t_new_pokemon));
			new_snd->nombre_pokemon = string_duplicate(
					new_receive->nombre_pokemon);
			new_snd->id = 28;
			new_snd->id_correlacional = new_receive->id_correlacional;
			new_snd->cantidad = new_receive->cantidad;
			new_snd->tamanio_nombre = strlen(new_snd->nombre_pokemon) + 1;
			new_snd->pos_x = new_receive->pos_x;
			new_snd->pos_y = new_receive->pos_y;
			new_protocol = NEW_POKEMON;
			broker_logger_info("NEW SENT TO GC");
			for (int i = 0; i < list_size(new_queue); i++) {
				t_subscribe_nodo* node = list_get(new_queue, i);
				if (node->endtime != -1) {
					if (time(NULL) >= node->endtime) {
						t_empty* noop = malloc(sizeof(t_empty));
						t_protocol noop_protocol = NOOP;
						utils_serialize_and_send(node->f_desc, noop_protocol, noop);
						list_remove(new_queue, i);
					} else {
						utils_serialize_and_send(node->f_desc, new_protocol,
								new_snd);
					}
				} else {
					utils_serialize_and_send(node->f_desc, new_protocol,
							new_snd);
				}
			}
			//free(new_snd->nombre_pokemon);
			//free(new_snd);
			usleep(50000);
			break;
		}

			// From GB or GC
		case APPEARED_POKEMON: {
			broker_logger_info("APPEARED RECEIVED");
			t_appeared_pokemon *appeared_rcv = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID correlacional: %d",
					appeared_rcv->id_correlacional);
			broker_logger_info("Cantidad: %d", appeared_rcv->cantidad);
			broker_logger_info("Nombre Pokemon: %s",
					appeared_rcv->nombre_pokemon);
			broker_logger_info("Largo nombre: %d",
					appeared_rcv->tamanio_nombre);
			broker_logger_info("Posicion X: %d", appeared_rcv->pos_x);
			broker_logger_info("Posicion Y: %d", appeared_rcv->pos_y);
			usleep(50000);
			t_message_to_void *message_void = convert_to_void(protocol,
					appeared_rcv);
			int from = save_on_memory(message_void);
			get_from_memory(protocol, from, memory);
			//free(message_void->message);
			//			free(message_void);
			// To Team
			t_appeared_pokemon* appeared_snd = malloc(
					sizeof(t_appeared_pokemon));
			appeared_protocol = APPEARED_POKEMON;
			appeared_snd->nombre_pokemon = appeared_rcv->nombre_pokemon;
			appeared_snd->tamanio_nombre = appeared_rcv->tamanio_nombre;
			appeared_snd->id_correlacional = appeared_rcv->id_correlacional;
			appeared_snd->pos_x = appeared_rcv->pos_x;
			appeared_snd->pos_y = appeared_rcv->pos_y;
			appeared_snd->cantidad = appeared_rcv->cantidad;
			broker_logger_info("APPEARED SENT TO TEAM");
			for (int i = 0; i < list_size(appeared_queue); i++) {
				t_subscribe_nodo* node = list_get(appeared_queue, i);
				if (node->endtime != -1) {
					if (time(NULL) >= node->endtime) {
						t_empty* noop = malloc(sizeof(t_empty));
						t_protocol noop_protocol = NOOP;
						utils_serialize_and_send(node->f_desc, noop_protocol, noop);
						list_remove(appeared_queue, i);
					} else {
						utils_serialize_and_send(node->f_desc,
								appeared_protocol, appeared_snd);
					}
				} else {
					utils_serialize_and_send(node->f_desc, appeared_protocol,
							appeared_snd);
				}
			}
			//free(appeared_snd->nombre_pokemon);
			//free(appeared_snd);
			usleep(500000);
			break;
		}
			// From team
		case GET_POKEMON: {
			broker_logger_info("GET RECEIVED FROM TEAM");
			t_get_pokemon *get_rcv = utils_receive_and_deserialize(client_fd,
					protocol);
			broker_logger_info("ID correlacional: %d",
					get_rcv->id_correlacional);
			broker_logger_info("Nombre Pokemon: %s", get_rcv->nombre_pokemon);
			broker_logger_info("Largo nombre: %d", get_rcv->tamanio_nombre);

			usleep(50000);
			t_message_to_void *message_void = convert_to_void(protocol, get_rcv);
			int from = save_on_memory(message_void);
			get_from_memory(protocol, from, memory);
			//free(message_void->message);
			//free(message_void);
			// To GC
			t_get_pokemon* get_snd = malloc(sizeof(t_get_pokemon));
			get_snd->id_correlacional = get_rcv->id_correlacional;
			get_snd->nombre_pokemon = get_rcv->nombre_pokemon;
			get_snd->tamanio_nombre = strlen(get_snd->nombre_pokemon) + 1;
			get_protocol = GET_POKEMON;
			broker_logger_info("GET SENT TO GAMECARD");
			for (int i = 0; i < list_size(get_queue); i++) {
				t_subscribe_nodo* node = list_get(get_queue, i);
				if (node->endtime != -1) {
					if (time(NULL) >= node->endtime) {
						t_empty* noop = malloc(sizeof(t_empty));
						t_protocol noop_protocol = NOOP;
						utils_serialize_and_send(node->f_desc, noop_protocol, noop);
						list_remove(get_queue, i);
					} else {
						utils_serialize_and_send(node->f_desc, get_protocol,
								get_snd);
					}
				} else {
					utils_serialize_and_send(node->f_desc, get_protocol,
							get_snd);
				}
			}
			//free(get_snd->nombre_pokemon);
			//free(get_snd);
			//free(get_rcv->nombre_pokemon);
			//free(get_rcv);

			usleep(500000);
			break;
		}

			// From team
		case CATCH_POKEMON: {
			broker_logger_info("CATCH RECEIVED FROM TEAM");
			t_catch_pokemon *catch_rcv = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID correlacional: %d",
					catch_rcv->id_correlacional);
			broker_logger_info("ID Generado: %d", catch_rcv->id_gen);
			broker_logger_info("Nombre Pokemon: %s", catch_rcv->nombre_pokemon);
			broker_logger_info("Largo nombre: %d", catch_rcv->tamanio_nombre);
			broker_logger_info("Posicion X: %d", catch_rcv->pos_x);
			broker_logger_info("Posicion Y: %d", catch_rcv->pos_y);
			usleep(50000);
			t_message_to_void *message_void = convert_to_void(protocol, catch_rcv);
			int from = save_on_memory(message_void);
			get_from_memory(protocol, from, memory);

			//free(message_void->message);
			//free(message_void);
			// To GC
			t_catch_pokemon* catch_send = malloc(sizeof(t_catch_pokemon));
			catch_send->id_correlacional = catch_rcv->id_correlacional;
			catch_send->nombre_pokemon = string_duplicate(catch_rcv->nombre_pokemon);
			catch_send->pos_x = catch_rcv->pos_x;
			catch_send->pos_y = catch_rcv->pos_y;
			catch_send->tamanio_nombre = strlen(catch_rcv->nombre_pokemon) + 1;
			catch_send->id_gen = uuid;
			uuid++;
			catch_protocol = CATCH_POKEMON;
			broker_logger_info("CATCH SENT TO GC");
			for (int i = 0; i < list_size(catch_queue); i++) {
				t_subscribe_nodo* node = list_get(catch_queue, i);
				if (node->endtime != -1) {
					if (time(NULL) >= node->endtime) {
						t_empty* noop = malloc(sizeof(t_empty));
						t_protocol noop_protocol = NOOP;
						utils_serialize_and_send(node->f_desc, noop_protocol, noop);
						list_remove(catch_queue, i);
					} else {
						utils_serialize_and_send(node->f_desc, catch_protocol,
								catch_send);
					}
				} else {
					utils_serialize_and_send(node->f_desc, catch_protocol,
							catch_send);
				}
			}
			//free(catch_rcv->nombre_pokemon);
			//free(catch_rcv);

			//free(catch_send->nombre_pokemon);
			//free(catch_send);
			usleep(500000);
			break;
		}
			// From GC
		case LOCALIZED_POKEMON: {
			broker_logger_info("LOCALIZED RECEIVED FROM GC");
			t_localized_pokemon *loc_rcv = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID correlacional: %d",

			loc_rcv->id_correlacional);
			broker_logger_info("Nombre Pokemon: %s", loc_rcv->nombre_pokemon);
			broker_logger_info("Largo nombre: %d", loc_rcv->tamanio_nombre);
			broker_logger_info("Cant Elementos en lista: %d",
					loc_rcv->cant_elem);
			for (int el = 0; el < loc_rcv->cant_elem; el++) {
				t_position* pos = malloc(sizeof(t_position));
				pos = list_get(loc_rcv->posiciones, el);
				broker_logger_info("Position is (%d, %d)", pos->pos_x,
						pos->pos_y);
			}
			t_message_to_void *message_void = convert_to_void(protocol, loc_rcv);
			int from = save_on_memory(message_void);
			get_from_memory(protocol, from, memory);
			free(message_void->message);
					free(message_void);
			// To team
			t_localized_pokemon* loc_snd = malloc(sizeof(t_localized_pokemon));
			loc_snd->id_correlacional = loc_rcv->id_correlacional;
			loc_snd->nombre_pokemon = loc_rcv->nombre_pokemon;
			loc_snd->tamanio_nombre = strlen(loc_snd->nombre_pokemon) + 1;
			loc_snd->cant_elem = list_size(loc_rcv->posiciones);
			localized_protocol = LOCALIZED_POKEMON;
			broker_logger_info("LOCALIZED SENT TO TEAM");
			loc_snd->posiciones = loc_rcv->posiciones;
			for (int i = 0; i < list_size(localized_queue); i++) {
				t_subscribe_nodo* node = list_get(localized_queue, i);
				if (node->endtime != -1) {
					if (time(NULL) >= node->endtime) {
						t_empty* noop = malloc(sizeof(t_empty));
						t_protocol noop_protocol = NOOP;
						utils_serialize_and_send(node->f_desc, noop_protocol, noop);
						list_remove(localized_queue, i);
					} else {
						utils_serialize_and_send(node->f_desc,
								localized_protocol, loc_snd);
					}
				} else {
					utils_serialize_and_send(node->f_desc, localized_protocol,
							loc_snd);
				}
			}
			//free(loc_rcv->nombre_pokemon);
			//list_destroy(loc_rcv->posiciones);
			//free(loc_rcv);
			//free(loc_snd->nombre_pokemon);
			//list_destroy(loc_snd->posiciones);
			//free(loc_snd);
			usleep(50000);
			break;
		}

			// From Team or GC
		case SUBSCRIBE: {
			broker_logger_info("SUBSCRIBE RECEIVED");
			t_subscribe *sub_rcv = utils_receive_and_deserialize(client_fd,
					protocol);
			char * ip = string_duplicate(sub_rcv->ip);
			broker_logger_info("Puerto Recibido: %d", sub_rcv->puerto);
			broker_logger_info("IP Recibido: %s", ip);
			sub_rcv->f_desc = client_fd;
			// TODO: Check dictionary
			search_queue(sub_rcv);
			//free(sub_rcv->ip);
			//free(sub_rcv);
			usleep(50000);
			break;
		}

			// From GC or GB
		case CAUGHT_POKEMON: {
			broker_logger_info("CAUGHT RECEIVED");
			t_caught_pokemon *caught_rcv = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID correlacional: %d",
					caught_rcv->id_correlacional);
			broker_logger_info("ID mensaje: %d", caught_rcv->id_msg);
			broker_logger_info("Resultado (0/1): %d", caught_rcv->result);
			usleep(50000);
			t_message_to_void *message_void = convert_to_void(protocol, caught_rcv);
			int from = save_on_memory(message_void);
			get_from_memory(protocol, from, memory);
			free(message_void->message);
			free(message_void);
			// To Team
			t_caught_pokemon* caught_snd = malloc(sizeof(t_caught_pokemon));
			caught_snd->id_correlacional = caught_rcv->id_correlacional;
			caught_snd->id_msg = caught_rcv->id_msg;
			caught_snd->result = caught_rcv->result;
			caught_protocol = CAUGHT_POKEMON;
			broker_logger_info("CAUGHT SENT TO TEAM");
			for (int i = 0; i < list_size(caught_queue); i++) {
				t_subscribe_nodo* node = list_get(caught_queue, i);
				if (node->endtime != -1) {
					if (time(NULL) >= node->endtime) {
						t_empty* noop = malloc(sizeof(t_empty));
						t_protocol noop_protocol = NOOP;
						utils_serialize_and_send(node->f_desc, noop_protocol, noop);
						list_remove(caught_queue, i);
					} else {
						utils_serialize_and_send(node->f_desc, caught_protocol,
								caught_snd);
					}
				} else {
					utils_serialize_and_send(node->f_desc, caught_protocol,
							caught_snd);
				}
			}
			free(caught_snd);
			free(caught_rcv);

			usleep(50000);
			break;
		}

		default:
			break;
		}
	}
}

void initialize_queue() {
	get_queue = list_create();
	appeared_queue = list_create();
	new_queue = list_create();
	caught_queue = list_create();
	catch_queue = list_create();
	localized_queue = list_create();
}

t_subscribe_nodo* check_already_subscribed(char *ip, uint32_t puerto,
		t_list *list) {
	int find_subscribed(t_subscribe_nodo *nodo) {
		return (strcmp(ip, nodo->ip) == 0 && (puerto == nodo->puerto));
	}

	return list_find(list, (void*) find_subscribed);
}

void add_to(t_list *list, t_subscribe* subscriber) {

	t_subscribe_nodo* node = check_already_subscribed(subscriber->ip,
			subscriber->puerto, list);
	if (node == NULL) {
		t_subscribe_nodo *nodo = malloc(sizeof(t_subscribe_nodo));
		nodo->ip = string_duplicate(subscriber->ip);
		nodo->puerto = subscriber->puerto;
		// Sync uid_subscribe var
		nodo->id = uid_subscribe;
		uid_subscribe++;

		if (subscriber->proceso == GAME_BOY) {
			nodo->endtime = time(NULL) + subscriber->seconds;
		} else {
			nodo->endtime = -1;
		}

		nodo->f_desc = subscriber->f_desc;
		list_add(list, nodo);
	} else {
		broker_logger_info("Ya esta Subscripto");
		node->f_desc = subscriber->f_desc;
	}
}

void search_queue(t_subscribe *subscriber) {

	switch (subscriber->cola) {
	case NEW_QUEUE: {
		broker_logger_info("Subscripto IP: %s, PUERTO: %d,  a Cola NEW ",
				subscriber->ip, subscriber->puerto);
		add_to(new_queue, subscriber);
		break;
	}
	case CATCH_QUEUE: {
		broker_logger_info("Subscripto IP: %s, PUERTO: %d,  a Cola CATCH ",
				subscriber->ip, subscriber->puerto);
		add_to(catch_queue, subscriber);
		break;
	}
	case CAUGHT_QUEUE: {
		broker_logger_info("Subscripto IP: %s, PUERTO: %d,  a Cola CAUGHT ",
				subscriber->ip, subscriber->puerto);
		add_to(caught_queue, subscriber);
		break;
	}
	case GET_QUEUE: {
		broker_logger_info("Subscripto IP: %s, PUERTO: %d,  a Cola GET ",
				subscriber->ip, subscriber->puerto);
		add_to(get_queue, subscriber);
		break;
	}
	case LOCALIZED_QUEUE: {
		broker_logger_info("Subscripto IP: %s, PUERTO: %d,  a Cola LOCALIZED ",
				subscriber->ip, subscriber->puerto);
		add_to(localized_queue, subscriber);
		break;
	}
	case APPEARED_QUEUE: {
		broker_logger_info("Subscripto IP: %s, PUERTO: %d,  a Cola APPEARED ",
				subscriber->ip, subscriber->puerto);
		add_to(appeared_queue, subscriber);
		break;
	}

	}
}

void broker_exit() {
	socket_close_conection(broker_socket);
	broker_config_free();
	broker_logger_destroy();
}

t_message_to_void *convert_to_void(t_protocol protocol, void *package_recv) {

	int offset = 0;
	t_message_to_void* message_to_void = malloc(sizeof(t_message_to_void));
	message_to_void->size_message = 0;
	switch (protocol) {
	case NEW_POKEMON: {
		broker_logger_info("NEW RECEIVED, CONVERTING to VOID*..");
		t_new_pokemon *new_receive = (t_new_pokemon*) package_recv;
		message_to_void->message = malloc(
				new_receive->tamanio_nombre + sizeof(uint32_t) * 4);
		memcpy(message_to_void->message + offset, &new_receive->tamanio_nombre,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, new_receive->nombre_pokemon,
				new_receive->tamanio_nombre);
		offset += new_receive->tamanio_nombre;
		memcpy(message_to_void->message + offset, &new_receive->pos_x,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, &new_receive->pos_y,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, &new_receive->cantidad,
				sizeof(uint32_t));
		message_to_void->size_message = new_receive->tamanio_nombre
				+ sizeof(uint32_t) * 4;

		break;
	}

		// From GB or GC
	case APPEARED_POKEMON: {
		broker_logger_info("APPEARED RECEIVED, CONVERTING TO VOID*..");

		t_appeared_pokemon *appeared_rcv = (t_appeared_pokemon*) package_recv;
		message_to_void->message = malloc(
				appeared_rcv->tamanio_nombre + sizeof(uint32_t) * 3);

		memcpy(message_to_void->message + offset, &appeared_rcv->tamanio_nombre,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, appeared_rcv->nombre_pokemon,
				appeared_rcv->tamanio_nombre);
		offset += appeared_rcv->tamanio_nombre;
		memcpy(message_to_void->message + offset, &appeared_rcv->pos_x,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, &appeared_rcv->pos_y,
				sizeof(uint32_t));

		message_to_void->size_message = appeared_rcv->tamanio_nombre
				+ sizeof(uint32_t) * 3;
		break;
	}
		// From team
	case GET_POKEMON: {
		broker_logger_info("GET RECEIVED, CONVERTING TO VOID*..");
		t_get_pokemon *get_rcv = (t_get_pokemon*) package_recv;
		message_to_void->message = malloc(
				get_rcv->tamanio_nombre + sizeof(uint32_t));

		memcpy(message_to_void->message + offset, &get_rcv->tamanio_nombre,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, get_rcv->nombre_pokemon,
				get_rcv->tamanio_nombre);
		message_to_void->size_message = get_rcv->tamanio_nombre
				+ sizeof(uint32_t);
		break;
	}

		// From team
	case CATCH_POKEMON: {
		broker_logger_info("CATCH RECEIVED, CONVERTING TO VOID*..");
		t_catch_pokemon *catch_rcv = (t_catch_pokemon*) package_recv;
		message_to_void->message = malloc(
				catch_rcv->tamanio_nombre + sizeof(uint32_t) * 3);

		memcpy(message_to_void->message + offset, &catch_rcv->tamanio_nombre,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, catch_rcv->nombre_pokemon,
				catch_rcv->tamanio_nombre);
		offset += catch_rcv->tamanio_nombre;
		memcpy(message_to_void->message + offset, &catch_rcv->pos_x,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, &catch_rcv->pos_y,
				sizeof(uint32_t));

		message_to_void->size_message = catch_rcv->tamanio_nombre
				+ sizeof(uint32_t) * 3;
		break;
	}
		// From GC
	case LOCALIZED_POKEMON: {
		broker_logger_info("LOCALIZED RECEIVED, CONVERTING TO VOID*..");
		t_localized_pokemon *loc_rcv = (t_localized_pokemon*) package_recv;
		message_to_void->message = malloc(
				loc_rcv->tamanio_nombre + sizeof(uint32_t)
				+sizeof(uint32_t)+ sizeof(uint32_t) * 2 * loc_rcv->cant_elem);

		memcpy(message_to_void->message + offset, &loc_rcv->tamanio_nombre,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(message_to_void->message + offset, loc_rcv->nombre_pokemon,
				loc_rcv->tamanio_nombre);
		offset += loc_rcv->tamanio_nombre;
		memcpy(message_to_void->message + offset, &loc_rcv->cant_elem,
				sizeof(uint32_t));
		for (int i = 0; i < loc_rcv->cant_elem; i++) {
			offset += sizeof(uint32_t);
			t_position *pos = list_get(loc_rcv->posiciones, i);
			memcpy(message_to_void->message + offset, &pos->pos_x,
					sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(message_to_void->message + offset, &pos->pos_y,
					sizeof(uint32_t));

		}
		message_to_void->size_message = loc_rcv->tamanio_nombre
				+ sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) * 2 * loc_rcv->cant_elem;
		break;
	}

	case CAUGHT_POKEMON: {
		broker_logger_info("CAUGHT RECEIVED, CONVERTING TO VOID*..");
		t_caught_pokemon *caught_rcv = (t_caught_pokemon*) package_recv;
		message_to_void->message = malloc(sizeof(uint32_t));

		memcpy(message_to_void->message, &caught_rcv->result, sizeof(uint32_t));
		message_to_void->size_message = sizeof(uint32_t);
		break;
	}

	}
	return message_to_void;

}

void *get_from_memory(t_protocol protocol, int posicion, void *message) {

	int offset = posicion;

	switch (protocol) {
	case NEW_POKEMON: {
		broker_logger_info("GETTING \"NEW\" MESSAGE FROM  MEMORY..");
		t_new_pokemon *new_receive = malloc(sizeof(t_new_pokemon));


		memcpy(&new_receive->tamanio_nombre, message + offset,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);
		new_receive->nombre_pokemon = malloc(new_receive->tamanio_nombre);
		memcpy(new_receive->nombre_pokemon, message + offset,
				new_receive->tamanio_nombre);
		offset += new_receive->tamanio_nombre;

		memcpy(&new_receive->pos_x, message + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&new_receive->pos_y, message + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&new_receive->cantidad, message + offset, sizeof(uint32_t));

		broker_logger_info("******************************************");
		broker_logger_info("RECEIVED:");
		broker_logger_info("Pokemon: %s", new_receive->nombre_pokemon);
		broker_logger_info("Name length: %d", new_receive->tamanio_nombre);
		broker_logger_info("X Axis position: %d", new_receive->pos_x);
		broker_logger_info("Y Axis position: %d", new_receive->pos_y);
		broker_logger_info("Quantity: %d", new_receive->cantidad);

		return new_receive;

	}

		// From GB or GC
	case APPEARED_POKEMON: {
		broker_logger_info("GETTING \"APPEARED\" MESSAGE FROM  MEMORY..");

		t_appeared_pokemon *appeared_rcv = malloc(sizeof(t_appeared_pokemon));


		memcpy(&appeared_rcv->tamanio_nombre, message + offset,
				sizeof(uint32_t));
		offset += sizeof(uint32_t);

		appeared_rcv->nombre_pokemon = malloc(appeared_rcv->tamanio_nombre);
		memcpy(appeared_rcv->nombre_pokemon, message + offset,
				appeared_rcv->tamanio_nombre);

		offset += appeared_rcv->tamanio_nombre;
		memcpy(&appeared_rcv->pos_x, message + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&appeared_rcv->pos_y, message + offset, sizeof(uint32_t));

		broker_logger_info("******************************************");
		broker_logger_info("RECEIVED:");
		broker_logger_info("Pokemon: %s", appeared_rcv->nombre_pokemon);
		broker_logger_info("Name length: %d", appeared_rcv->tamanio_nombre);
		broker_logger_info("X Axis position: %d", appeared_rcv->pos_x);
		broker_logger_info("Y Axis position: %d", appeared_rcv->pos_y);

		return appeared_rcv;
	}
		// From team
	case GET_POKEMON: {
		broker_logger_info("GETING \"GET\" MESSAGE FROM  MEMORY..");
		t_get_pokemon* get_rcv = malloc(sizeof(t_get_pokemon));


		memcpy(&get_rcv->tamanio_nombre, message + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		get_rcv->nombre_pokemon = malloc(get_rcv->tamanio_nombre);
		memcpy(get_rcv->nombre_pokemon, message + offset,
				get_rcv->tamanio_nombre);

		broker_logger_info("******************************************");
		broker_logger_info("RECEIVED:");
		broker_logger_info("Pokemon: %s", get_rcv->nombre_pokemon);
		broker_logger_info("Name length: %d", get_rcv->tamanio_nombre);

		return get_rcv;
	}

		// From team
	case CATCH_POKEMON: {
		broker_logger_info("GETTING \"CATCH\" MESSAGE FROM  MEMORY..");
		t_catch_pokemon *catch_rcv = malloc(sizeof(t_catch_pokemon));
		memcpy(&catch_rcv->tamanio_nombre, message + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		catch_rcv->nombre_pokemon = malloc(catch_rcv->tamanio_nombre);
		memcpy(catch_rcv->nombre_pokemon, message + offset,
				catch_rcv->tamanio_nombre);
		offset += catch_rcv->tamanio_nombre;;
		memcpy(&catch_rcv->pos_x, message + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&catch_rcv->pos_y, message + offset, sizeof(uint32_t));

		return catch_rcv;
	}
		// From GC
	case LOCALIZED_POKEMON: {
		broker_logger_info("GETTING \"LOCALIZED\" MESSAGE FROM MEMORY..");
		t_localized_pokemon *loc_rcv = malloc(sizeof(t_localized_pokemon));

		memcpy(&loc_rcv->tamanio_nombre, message + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		loc_rcv->nombre_pokemon = malloc(loc_rcv->tamanio_nombre);
		memcpy(loc_rcv->nombre_pokemon, message + offset,
				loc_rcv->tamanio_nombre);

		loc_rcv->posiciones = list_create();
		offset += loc_rcv->tamanio_nombre;
		memcpy(&loc_rcv->cant_elem, message + offset, sizeof(uint32_t));

		broker_logger_info("GETTING \"LOCALIZED\" POKEMON NAME SIZE %d",loc_rcv->tamanio_nombre);
		broker_logger_info("GETTING \"LOCALIZED\" POKEMON NAME %s",loc_rcv->nombre_pokemon);
		broker_logger_info("GETTING \"LOCALIZED\" POKEMON CANT ELEMENTS %d",loc_rcv->cant_elem);
		for (int i = 0; i < loc_rcv->cant_elem; i++) {
			offset += sizeof(uint32_t);
			t_position* pos = malloc(sizeof(t_position));
			memcpy(&pos->pos_x, message + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(&pos->pos_y, message + offset, sizeof(uint32_t));
			list_add(loc_rcv->posiciones, pos);
			broker_logger_info("GETTING \"LOCALIZED\" POKEMON POS X %d",pos->pos_x);
			broker_logger_info("GETTING \"LOCALIZED\" POKEMON POS Y %d",pos->pos_y);

		}
		return loc_rcv;
	}

	case CAUGHT_POKEMON: {
		broker_logger_info("GETTING \"CAUGHT\" MESSAGE FROM MEMORY..");
		t_caught_pokemon *caught_rcv = malloc(sizeof(t_caught_pokemon));

		memcpy(&caught_rcv->result, message, sizeof(uint32_t));
		if (caught_rcv->result){
			broker_logger_info("CAUGHT POKEMON OK ");
		}
		else{
			broker_logger_info("CAUGHT POKEMON FAILED");
		}

		return caught_rcv;
	}
	}
	return NULL;

}


int save_on_memory(t_message_to_void *message_void){
	pthread_mutex_lock(&mpointer);
	int from = pointer;
	pointer += message_void->size_message;
	memcpy(memory + from,message_void->message,message_void->size_message);
	pthread_mutex_unlock(&mpointer);
	return from;
}


