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

void team_connect_to() {

}

//Esto iria en un hilo
void team_retry_connect()
{
	while (true)
	{
		utils_delay(team_config->tiempo_reconexion);
		team_connect_to();
	}
}

void team_init() {

	team_planner_init();
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
	pthread_t tid;
	pthread_t tid2;
	pthread_t tid3;

	t_cola cola = NEW_QUEUE;
	team_logger_info(
			"Creando un hilo para subscribirse a la cola new del broker %d");

	pthread_create(&tid, NULL, (void*) subscribe_to, (void*) &cola);
	pthread_detach(tid);

	team_logger_info(
			"Creando un hilo para subscribirse a la cola catch del broker %d");

	usleep(500000);
	cola = CATCH_QUEUE;
	pthread_create(&tid2, NULL, (void*) subscribe_to, (void*) &cola);
	pthread_detach(tid2);
	usleep(500000);

	pthread_create(&tid3, NULL, (void*) send_message_test, NULL);
	pthread_detach(tid3);
	for (;;)
		;

}

void send_message_test() {
	int broker_fd_send = socket_connect_to_server(team_config->ip_broker,
			team_config->puerto_broker);

	if (broker_fd_send < 0) {
		team_logger_warn("No se pudo conectar con BROKER");
		socket_close_conection(broker_fd_send);
	} else {
		team_logger_info("Conexion con BROKER establecida correctamente!");

		t_protocol ack_protocol;
		t_protocol new_protocol;
		t_protocol catch_protocol;
		t_protocol get_protocol;
		t_protocol appeared_protocol;
		t_protocol localized_protocol;
		t_protocol subscribe_protocol;

		// To broker
		t_ack* ack_snd = malloc(sizeof(t_ack));
		ack_snd->id = 1;
		ack_snd->id_correlacional = 1;
		ack_protocol = ACK;
		team_logger_info("Envio de ACK!");
		utils_serialize_and_send(broker_fd_send, ack_protocol, ack_snd);

		usleep(500000);

		// To broker
		t_subscribe* sub_snd = malloc(sizeof(t_subscribe));
		subscribe_protocol = SUBSCRIBE;
		sub_snd->ip = string_duplicate(team_config->ip_team);
		sub_snd->puerto = team_config->puerto_team;
		sub_snd->proceso = TEAM;
		sub_snd->cola = GET_QUEUE;
		utils_serialize_and_send(broker_fd_send, subscribe_protocol, sub_snd);
		team_logger_info(sub_snd->ip);
		team_logger_info("%d", sub_snd->puerto);

		usleep(500000);

		// To broker
		t_get_pokemon* get_send = malloc(sizeof(t_get_pokemon));
		get_send->id_correlacional = 19;
		get_send->nombre_pokemon = string_duplicate("Aerodactyl");
		get_send->tamanio_nombre = strlen(get_send->nombre_pokemon) + 1;
		get_protocol = GET_POKEMON;
		team_logger_info("Get sent");
		utils_serialize_and_send(broker_fd_send, get_protocol, get_send);

		usleep(500000);
	}
}

void subscribe_to(void *arg) {

	t_cola cola = *((int *) arg);
	int new_broker_fd = socket_connect_to_server(team_config->ip_broker,
			team_config->puerto_broker);

	if (new_broker_fd < 0) {
		team_logger_warn("No se pudo conectar con BROKER");
		socket_close_conection(new_broker_fd);
	} else {
		team_logger_info("Conexion con BROKER establecida correctamente!");
	}

	t_subscribe* sub_snd = malloc(sizeof(t_subscribe));

	t_protocol subscribe_protocol = SUBSCRIBE;
	sub_snd->ip = string_duplicate(team_config->ip_team);
	sub_snd->puerto = team_config->puerto_team;
	sub_snd->proceso = TEAM;
	sub_snd->cola = cola;
	utils_serialize_and_send(new_broker_fd, subscribe_protocol, sub_snd);
	usleep(500000);

	recv_broker(new_broker_fd);

}

void *recv_broker(int new_broker_fd) {
	int protocol;

	int received_bytes = recv(new_broker_fd, &protocol, sizeof(int), 0);

	if (received_bytes <= 0) {
		team_logger_error("Error al recibir mensaje");
		return NULL;
	}
	switch (protocol) {
	case CAUGHT_POKEMON: {
		team_logger_info("Caught received");
		t_caught_pokemon *caught_rcv = utils_receive_and_deserialize(
				new_broker_fd, protocol);
		team_logger_info("ID correlacional: %d", caught_rcv->id_correlacional);
		team_logger_info("ID mensaje: %d", caught_rcv->id_msg);
		team_logger_info("Resultado (0/1): %d", caught_rcv->result);
		usleep(50000);
		break;
	}

	case LOCALIZED_POKEMON: {
		team_logger_info("Localized received");
		t_localized_pokemon *loc_rcv = utils_receive_and_deserialize(
				new_broker_fd, protocol);
		team_logger_info("ID correlacional: %d",

		loc_rcv->id_correlacional);
		team_logger_info("Nombre Pokemon: %s", loc_rcv->nombre_pokemon);
		team_logger_info("Largo nombre: %d", loc_rcv->tamanio_nombre);
		team_logger_info("Cant Elementos en lista: %d", loc_rcv->cant_elem);
		for (int el = 0; el < loc_rcv->cant_elem; el++) {
			t_position* pos = malloc(sizeof(t_position));
			pos = list_get(loc_rcv->posiciones, el);
			team_logger_info("Position is (%d, %d)", pos->pos_x, pos->pos_y);
		}
		usleep(500000);
		break;
	}

	case APPEARED_POKEMON: {
		team_logger_info("Appeared received");
		t_appeared_pokemon *appeared_rcv = utils_receive_and_deserialize(
				new_broker_fd, protocol);
		team_logger_info("ID correlacional: %d",
				appeared_rcv->id_correlacional);
		team_logger_info("Cantidad: %d", appeared_rcv->cantidad);
		team_logger_info("Nombre Pokemon: %s", appeared_rcv->nombre_pokemon);
		team_logger_info("Largo nombre: %d", appeared_rcv->tamanio_nombre);
		team_logger_info("Posicion X: %d", appeared_rcv->pos_x);
		team_logger_info("Posicion Y: %d", appeared_rcv->pos_y);
		usleep(50000);
		break;
	}
	}
	return NULL;
}

void team_server_init() {

	team_socket = socket_create_listener(team_config->ip_team,
			team_config->puerto_team);
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
	//socket_close_conection(broker_fd);
	team_config_free();
	team_logger_destroy();
}
