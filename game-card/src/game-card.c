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

void game_card_init()
{
	game_card_fd = socket_connect_to_server(game_card_config->ip_broker, game_card_config->puerto_broker);

	if (game_card_fd < 0)
	{
		game_card_logger_warn("No se pudo conectar con BROKER");
		socket_close_conection(game_card_fd);
	}
	else
	{
		game_card_logger_info("Conexion con BROKER establecida correctamente!");
	}
	game_card_logger_info("Inicando GAME CARD..");
	gcfs_create_structs();
	game_card_server_init();
}

void game_card_server_init()
{
	game_card_socket = socket_create_listener(LOCAL_IP, LOCAL_PORT);
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

	for(;;)
	{
		int accepted_fd;
		pthread_t tid;

		if((accepted_fd = accept(game_card_socket, (struct sockaddr *) &client_info, &addrlen)) != -1)
		{
			pthread_create(&tid, NULL, (void*) handle_connection, (void*) &accepted_fd);
			pthread_detach(tid);
			game_card_logger_info("Creando un hilo para atender una conexión en el socket %d", accepted_fd);
		}
		else
		{
			game_card_logger_error("Error al conectar con un cliente");
		}
	}
}

static void *handle_connection(void *arg)
{
	int client_fd = *((int *) arg);

	game_card_logger_info("Conexion establecida con cliente GAME BOY: %d", client_fd);
	int received_bytes;
	int protocol;
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
			case HANDSHAKE:
				game_card_logger_info("Recibi una nueva conexion");
				break;
			default:
				break;
		}
	}
}

void game_card_exit()
{
	socket_close_conection(game_card_fd);
	socket_close_conection(game_card_socket);
	gcfs_free_bitmap();
	game_card_config_free();
	game_card_logger_destroy();
}
