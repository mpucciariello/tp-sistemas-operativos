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

		t_ack* ack_snd = malloc(sizeof(t_ack));
		ack_snd->id = 1;
		ack_snd->id_correlacional = 1;
		t_protocol ack_protocol = ACK;
		team_logger_info("Envio de ACK!");
		utils_serialize_and_send(team_fd, ack_protocol, ack_snd);

		t_new_pokemon* new_snd = malloc(sizeof(t_new_pokemon));
		new_snd->pokemon = string_duplicate("pikachu");
		new_snd->id = 1;
		new_snd->id_correlacional = 1;
		new_snd->largo = 8;
		new_snd->x = 1;
		new_snd->y = 1;
		t_protocol new_protocol = NEW_POKEMON;
		team_logger_info("Envio de New Pokemon");
		utils_serialize_and_send(team_fd, new_protocol, new_snd);

		t_appeared_pokemon* appeared_snd = malloc(sizeof(t_appeared_pokemon));
		t_protocol appeared_protocol = APPEARED_POKEMON;
		appeared_snd->pokemon = string_duplicate("Raichu");
		appeared_snd->largo = 7;
		appeared_snd->id_correlacional = 2;
		appeared_snd->x = 1;
		appeared_snd->y = 1;
		appeared_snd->cantidad = 1;
		team_logger_info("Envio de APPEARED Pokemon");
		utils_serialize_and_send(team_fd, appeared_protocol, appeared_snd);

	}
	team_logger_info("Inicando TEAM..");
	team_server_init();
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
		case HANDSHAKE:
			team_logger_info("Recibi una nueva conexion");
			break;
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
