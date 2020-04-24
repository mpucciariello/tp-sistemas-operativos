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
	team_fd = socket_connect_to_server(team_config->ip_broker,
			team_config->puerto_broker);

	if (team_fd < 0) {
		team_logger_warn("No se pudo conectar con BROKER");
		socket_close_conection(team_fd);
	} else {
		team_logger_info("Conexion con BROKER establecida correctamente!");

		t_protocol ack_protocol;
		t_protocol new_protocol;
		t_protocol catch_protocol;
		t_protocol get_protocol;
		t_protocol appeared_protocol;
		t_protocol localized_protocol;
		t_protocol subscribe_protocol;
		t_list* positions = list_create();
		t_position *pos = malloc(sizeof(t_position));
		pos->pos_x = 21;
		pos->pos_y = 8;
		t_position *pos2 = malloc(sizeof(t_position));
		pos2->pos_x = 2;
		pos2->pos_y = 8;
		list_add(positions, pos);
		list_add(positions, pos2);


		// To broker
		t_ack* ack_snd = malloc(sizeof(t_ack));
		ack_snd->id = 1;
		ack_snd->id_correlacional = 1;
		ack_protocol = ACK;
		team_logger_info("Envio de ACK!");
		utils_serialize_and_send(team_fd, ack_protocol, ack_snd);

		sleep(1);

		// For testing purposes, should not be here
		t_new_pokemon* new_snd = malloc(sizeof(t_new_pokemon));
		new_snd->nombre_pokemon = string_duplicate("pikachu");
		new_snd->id = 1;
		new_snd->id_correlacional = 1;
		new_snd->tamanio_nombre = 8;
		new_snd->pos_x = 1;
		new_snd->pos_y = 1;
		new_protocol = NEW_POKEMON;
		team_logger_info("Envio de New Pokemon");
		utils_serialize_and_send(team_fd, new_protocol, new_snd);

		sleep(1);

		// For testing purposes, should not be here
		t_appeared_pokemon* appeared_snd = malloc(sizeof(t_appeared_pokemon));
		appeared_protocol = APPEARED_POKEMON;
		appeared_snd->nombre_pokemon = string_duplicate("Raichu");
		appeared_snd->tamanio_nombre = 7;
		appeared_snd->id_correlacional = 2;
		appeared_snd->pos_x = 1;
		appeared_snd->pos_y = 1;
		appeared_snd->cantidad = 1;
		team_logger_info("Envio de APPEARED Pokemon");
		utils_serialize_and_send(team_fd, appeared_protocol, appeared_snd);

		sleep(1);

		// To broker
		t_catch_pokemon* catch_send = malloc(sizeof(t_catch_pokemon));
		catch_send->id_correlacional = 111;
		catch_send->nombre_pokemon = string_duplicate("Weepinbell");
		catch_send->pos_x = 17;
		catch_send->pos_y = 8;
		catch_send->tamanio_nombre = strlen(catch_send->nombre_pokemon) + 1;
		catch_send->id_gen = -1;
		catch_protocol = CATCH_POKEMON;
		team_logger_info("Catch sent");
		utils_serialize_and_send(team_fd, catch_protocol, catch_send);



		sleep(1);

		// To broker
		t_subscribe* sub_snd = malloc(sizeof(t_subscribe));

		subscribe_protocol = SUBSCRIBE;
		sub_snd->ip = string_duplicate(team_config->ip_team);
		sub_snd->puerto = team_config->puerto_team;
		sub_snd->proceso = TEAM;
		sub_snd->cola = GET_QUEUE;
		utils_serialize_and_send(team_fd, subscribe_protocol, sub_snd);
		sleep(1);
		utils_serialize_and_send(team_fd, subscribe_protocol, sub_snd);
		sleep(1);
		// Fix n remove thread sleep
		sleep(1);

		// To broker
		t_get_pokemon* get_send = malloc(sizeof(t_get_pokemon));
		get_send->id_correlacional = 19;
		get_send->nombre_pokemon = string_duplicate("Aerodactyl");
		get_send->tamanio_nombre = strlen(get_send->nombre_pokemon) + 1;
		get_protocol = GET_POKEMON;
		team_logger_info("Get sent");
		utils_serialize_and_send(team_fd, get_protocol, get_send);

		sleep(1);

		// To broker
		t_localized_pokemon* loc_snd = malloc(sizeof(t_localized_pokemon));
		loc_snd->id_correlacional = 77;
		loc_snd->nombre_pokemon = string_duplicate("Celebi");
		loc_snd->tamanio_nombre = strlen(loc_snd->nombre_pokemon) + 1;
		loc_snd->cant_elem = list_size(positions);
		localized_protocol = LOCALIZED_POKEMON;
		team_logger_info("subscribe sent");
		loc_snd->posiciones = positions;
		utils_serialize_and_send(team_fd, localized_protocol, loc_snd);



		team_logger_info("Iniciando TEAM..");
		team_server_init();
	}
}
void team_server_init() {

	team_socket = socket_create_listener(LOCAL_IP, LOCAL_PORT);
	if (team_socket < 0) {
		team_logger_error("Error al crear server");
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

		if ((accepted_fd = accept(team_socket, (struct sockaddr *) &client_info,
				&addrlen)) != -1) {

			pthread_create(&tid, NULL, (void*) handle_connection,
					(void*) &accepted_fd);
			pthread_detach(tid);
			team_logger_info(
					"Creando un hilo para atender una conexión en el socket %d",
					accepted_fd);
		} else {
			team_logger_error("Error al conectar con un cliente");
		}
	}
}
static void *handle_connection(void *arg) {
	int client_fd = *((int *) arg);

	team_logger_info("Conexion establecida con cliente GAME BOY: %d",
			client_fd);
	int received_bytes;
	int protocol;
	while (true) {
		received_bytes = recv(client_fd, &protocol, sizeof(int), 0);

		if (received_bytes <= 0) {
			team_logger_error("Error al recibir mensaje");
			return NULL;
		}
		switch (protocol) {


			// From Broker
		case LOCALIZED_POKEMON: {
			team_logger_info("Localized received");
			break;
		}

			// From Broker
		case CAUGHT_POKEMON: {
			team_logger_info("Caught received");
			break;
		}

		default:
			break;
		}
	}
}

void team_exit() {
	socket_close_conection(team_socket);
	socket_close_conection(team_fd);
	team_config_free();
	team_logger_destroy();
}
