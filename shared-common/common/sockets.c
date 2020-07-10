#include "sockets.h"

int socket_create_listener(char* ip, int port) {

	// Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	int sockfd;
	// información sobre mi dirección
	struct sockaddr_in my_addr;
	int yes = 1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
	}

	// Ordenación de bytes de la máquina
	my_addr.sin_family = AF_INET;
	// short, Ordenación de bytes de la red
	my_addr.sin_port = htons(port);
	// Rellenar con mi dirección IP
	my_addr.sin_addr.s_addr = inet_addr(ip);
	// Poner a cero el resto de la estructura
	memset(&(my_addr.sin_zero), '\0', 8);

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
	}

	return sockfd;
//
//	if (ip == NULL)
//		return -1;
//
//	struct addrinfo hints;
//	struct addrinfo *server_info;
//
//	memset(&hints, 0, sizeof(hints));
//
//	hints.ai_family = AF_UNSPEC;
//	hints.ai_flags = AI_PASSIVE;
//	hints.ai_socktype = SOCK_STREAM;
//	char* port_char = string_itoa(port);
//	getaddrinfo(ip, port_char, &hints, &server_info);
//	free(port_char);
//
//	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
//
//	int activated = 1;
//	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &activated, sizeof(activated));
//
//	if (server_socket == -1 || bind(server_socket, server_info->ai_addr, server_info->ai_addrlen) == -1) {
//		freeaddrinfo(server_info);
//		return -1;
//	}
//
//	freeaddrinfo(server_info);
//
//	if (listen(server_socket, BACKLOG) == -1)
//		return -1;
//	return server_socket;
}

int socket_connect_to_server(char* ip, int port) {
	if (ip == NULL)
		return -1;

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	char* port_char = string_itoa(port);
	getaddrinfo(ip, port_char, &hints, &server_info);
	free(port_char);

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int result = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return (result < 0 || server_socket == -1) ? -1 : server_socket;
}

int socket_accept_conection(int server_socket) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int client_socket = accept(server_socket, (struct sockaddr *) &addr, &addrlen);
	printf("%d", client_socket);
	if (client_socket < 0) {
		perror("Error al aceptar cliente");
		return -1;
	}
	return client_socket;
}

char* socket_get_ip(int fd) {
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	int res = getpeername(fd, (struct sockaddr *) &addr, &addr_size);
	if (res == -1)
		return NULL;
	char ip_node[20];
	strcpy(ip_node, inet_ntoa(addr.sin_addr));
	return strdup(ip_node);
}

void socket_close_conection(int socket_client) {
	if (socket_client > 0)
		close(socket_client);
}
