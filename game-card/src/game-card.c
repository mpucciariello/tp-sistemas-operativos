#include "game-card.h"

int main(int argc, char *argv[])
{
	if(game_card_load() < 0)
		return EXIT_FAILURE;
	game_card_init();
	game_card_exit();

	return EXIT_SUCCESS;
}

int game_card_load()
{
	int response = game_card_logger_create();
	if(response < 0)
		return response;

	response = game_card_config_load();
	if(response < 0)
	{
		game_card_logger_destroy();
		return response;
	}
	return 0;
}


void game_card_init(){	
		game_card_logger_info("Inicando GAME CARD..");
		gcfs_create_structs();

	    pthread_attr_t attrs;
		pthread_attr_init(&attrs);
		pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
		pthread_t tid;
		pthread_t tid2;
		pthread_t tid3;
		pthread_t tid4;
		t_cola new_queue;
		t_cola catch_queue;
		t_cola get_queue;

        game_card_logger_info("Creando un hilo para subscribirse a la cola NEW del broker %d");
		new_queue = NEW_QUEUE;
        pthread_create(&tid, NULL, (void*) game_card_retry_connect, (void*) &new_queue);
		pthread_detach(tid);
		usleep(500000);
		
        game_card_logger_info("Creando un hilo para subscribirse a la cola CATCH del broker %d");
		catch_queue = CATCH_QUEUE;
		pthread_create(&tid2, NULL, (void*) game_card_retry_connect, (void*) &catch_queue);
		pthread_detach(tid2);
		usleep(500000);

		game_card_logger_info("Creando un hilo para subscribirse a la cola GET del broker %d");
		get_queue = GET_QUEUE;
		pthread_create(&tid3, NULL, (void*) game_card_retry_connect, (void *) &get_queue);
		pthread_detach(tid3);
		usleep(500000);

		game_card_logger_info("Creando un hilo para poder al Gamecard en modo servidor");
		game_card_init_as_server();
		usleep(500000);

		for(;;);
}


void game_card_retry_connect(void* arg) {
	void* arg2 = arg;
	while (true) {
		is_connected = false;
		subscribe_to(arg2);
		utils_delay(game_card_config->tiempo_de_reintento_conexion);
	}
}

void subscribe_to(void *arg) {

	t_cola cola = *((int *) arg);
	int new_broker_fd = socket_connect_to_server(game_card_config->ip_broker,
			game_card_config->puerto_broker);

	if (new_broker_fd < 0) {
		game_card_logger_warn("No se pudo conectar con BROKER");
		socket_close_conection(new_broker_fd);
	} else {
		game_card_logger_info("Conexion con BROKER establecida correctamente!");
        t_subscribe* sub_snd = malloc(sizeof(t_subscribe));
        t_protocol subscribe_protocol = SUBSCRIBE;
        sub_snd->ip = string_duplicate(game_card_config->ip_game_card);
        sub_snd->puerto = game_card_config->puerto_game_card;
        sub_snd->proceso = GAME_CARD;
        sub_snd->cola = cola;
        sub_snd->f_desc = -1;
        utils_serialize_and_send(new_broker_fd, subscribe_protocol, sub_snd);
        handle_connection(new_broker_fd);
	}
}

void game_card_init_as_server() {
    int game_card_socket = socket_create_listener(game_card_config->ip_game_card, game_card_config->puerto_game_card);
	if(game_card_socket < 0)
	{
		game_card_logger_error("Error al crear server");
	}
	game_card_logger_info("Server creado correctamente!! Esperando conexiones...");
	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
	int accepted_fd;
	for(;;)
	{
		pthread_t tid;
		if((accepted_fd = accept(game_card_socket, (struct sockaddr *) &client_info, &addrlen)) != -1)
		{
			pthread_create(&tid, NULL, (void*) handle_connection, (void*) &accepted_fd);
			pthread_detach(tid);
			game_card_logger_info("Creando un hilo para atender una conexión en el socket %d", accepted_fd);
			usleep(500000);
		}
		else
		{
			game_card_logger_error("Error al conectar con un cliente");
		}
	}
}

void *handle_connection(int fd){
		int received_bytes;
		int protocol;
		int client_fd = fd;
		while(true)
		{
			received_bytes = recv(client_fd, &protocol, sizeof(int), 0);

			if(received_bytes <= 0)
			{
				game_card_logger_error("Error al recibir mensaje");
				return NULL;
			}
			switch(protocol)
			{
			case NEW_POKEMON: {
					game_card_logger_info("New received");
					t_new_pokemon *new_receive = utils_receive_and_deserialize(
							client_fd, protocol);
					game_card_logger_info("ID recibido: %d", new_receive->id);
					game_card_logger_info("ID Correlacional: %d",
							new_receive->id_correlacional);
					game_card_logger_info("Cantidad: %d", new_receive->cantidad);
					game_card_logger_info("Nombre Pokemon: %s", new_receive->nombre_pokemon);
					game_card_logger_info("Largo Nombre: %d", new_receive->tamanio_nombre);
					game_card_logger_info("Posicion X: %d", new_receive->pos_x);
					game_card_logger_info("Posicion Y: %d", new_receive->pos_y);
					usleep(100000);
					break;
				}
								// From team
				case GET_POKEMON: {
					game_card_logger_info("Get received");
					t_get_pokemon *get_rcv = utils_receive_and_deserialize(client_fd,
							protocol);
					game_card_logger_info("ID correlacional: %d",
							get_rcv->id_correlacional);
					game_card_logger_info("Nombre Pokemon: %s", get_rcv->nombre_pokemon);
					game_card_logger_info("Largo nombre: %d", get_rcv->tamanio_nombre);
					usleep(50000);
					break;
				}

					// From team
				case CATCH_POKEMON: {
					game_card_logger_info("Catch received");
					t_catch_pokemon *catch_rcv = utils_receive_and_deserialize(
							client_fd, protocol);
					game_card_logger_info("ID correlacional: %d",
							catch_rcv->id_correlacional);
					game_card_logger_info("ID Generado: %d", catch_rcv->id_gen);
					game_card_logger_info("Nombre Pokemon: %s", catch_rcv->nombre_pokemon);
					game_card_logger_info("Largo nombre: %d", catch_rcv->tamanio_nombre);
					game_card_logger_info("Posicion X: %d", catch_rcv->pos_x);
					game_card_logger_info("Posicion Y: %d", catch_rcv->pos_y);
					usleep(50000);
					break;
				}

				default:
				break;

			}
		}
}

void game_card_exit()
{
	socket_close_conection(game_card_fd);
	//socket_close_conection(game_card_socket);
	gcfs_free_bitmap();
	game_card_config_free();
	game_card_logger_destroy();
}