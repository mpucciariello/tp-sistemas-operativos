#include "broker.h"

int main(int argc, char *argv[]) {
	initialice_queue();
	if (broker_load() < 0)
		return EXIT_FAILURE;
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

	for (;;) {
		int accepted_fd;
		pthread_t tid;

		if ((accepted_fd = accept(broker_socket,
				(struct sockaddr *) &client_info, &addrlen)) != -1) {

			pthread_create(&tid, NULL, (void*) handle_connection,
					(void*) &accepted_fd);
			pthread_detach(tid);
			broker_logger_info(
					"Creando un hilo para atender una conexión en el socket %d",
					accepted_fd);
		} else {
			broker_logger_error("Error al conectar con un cliente");
		}
	}
}
static void *handle_connection(void *arg) {
	int client_fd = *((int *) arg);

	broker_logger_info("Conexion establecida con cliente: %d", client_fd);

	int received_bytes;
	int protocol;
	while (true) {
		received_bytes = recv(client_fd, &protocol, sizeof(int), 0);

		if (received_bytes <= 0) {
			broker_logger_error("Error al recibir mensaje");
			return NULL;
		}
		switch (protocol) {
		case ACK: {
			broker_logger_info("Ack received");
			t_ack *ack_receive = utils_receive_and_deserialize(client_fd,
					protocol);
			broker_logger_info("ID recibido: %d", ack_receive->id);
			broker_logger_info("ID correlacional %d",
					ack_receive->id_correlacional);

			break;
		}

		case NEW_POKEMON: {
			broker_logger_info("New received");
			t_new_pokemon *new_receive = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID recibido: %d", new_receive->id);
			broker_logger_info("ID Correlacional: %d",
					new_receive->id_correlacional);
			broker_logger_info("Cantidad: %d", new_receive->cantidad);
			broker_logger_info("Nombre Pokemon: %s", new_receive->nombre_pokemon);
			broker_logger_info("Largo Nombre: %d", new_receive->tamanio_nombre);
			broker_logger_info("Posicion X: %d", new_receive->pos_x);
			broker_logger_info("Posicion Y: %d", new_receive->pos_y);
			break;
		}
		case APPEARED_POKEMON: {
			broker_logger_info("Appeared received");
			t_appeared_pokemon *appeared_rcv = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID correlacional: %d",
					appeared_rcv->id_correlacional);
			broker_logger_info("Cantidad: %d", appeared_rcv->cantidad);
			broker_logger_info("Nombre Pokemon: %s", appeared_rcv->nombre_pokemon);
			broker_logger_info("Largo nombre: %d", appeared_rcv->tamanio_nombre);
			broker_logger_info("Posicion X: %d", appeared_rcv->pos_x);
			broker_logger_info("Posicion Y: %d", appeared_rcv->pos_y);
			break;
		}
			// From team
		case GET_POKEMON: {
			broker_logger_info("Get received");
			t_get_pokemon *get_rcv = utils_receive_and_deserialize(client_fd,
					protocol);
			broker_logger_info("ID correlacional: %d",
					get_rcv->id_correlacional);
			broker_logger_info("Nombre Pokemon: %s", get_rcv->nombre_pokemon);
			broker_logger_info("Largo nombre: %d", get_rcv->tamanio_nombre);
			break;
		}

			// From team
		case CATCH_POKEMON: {
			broker_logger_info("Catch received");
			t_catch_pokemon *catch_rcv = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID correlacional: %d",
					catch_rcv->id_correlacional);
			broker_logger_info("ID Generado: %d", catch_rcv->id_gen);
			broker_logger_info("Nombre Pokemon: %s", catch_rcv->nombre_pokemon);
			broker_logger_info("Largo nombre: %d", catch_rcv->tamanio_nombre);
			broker_logger_info("Posicion X: %d", catch_rcv->pos_x);
			broker_logger_info("Posicion Y: %d", catch_rcv->pos_y);
			break;
		}

			// From GC
		case LOCALIZED_POKEMON: {
			broker_logger_info("Localized received");
			t_localized_pokemon *loc_rcv = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID correlacional: %d",
					loc_rcv->id_correlacional);
			broker_logger_info("Nombre Pokemon: %s", loc_rcv->nombre_pokemon);
			broker_logger_info("Largo nombre: %d", loc_rcv->tamanio_nombre);
			broker_logger_info("Cant Elementos en lista: %d", loc_rcv->cant_elem);
			for(int el = 0; el < loc_rcv->cant_elem; el++) {
				t_position* pos = malloc(sizeof(t_position));
				pos = list_get(loc_rcv->posiciones, el);
				broker_logger_info("Position is (%d, %d)", pos->pos_x, pos->pos_y);
			}
			break;
		}
		case SUBSCRIBE: {
			broker_logger_info("SUBSCRIBE received");
			t_subscribe *sub_rcv = utils_receive_and_deserialize(
								client_fd, protocol);
			char * ip= string_duplicate(sub_rcv->ip);
			broker_logger_info("Puerto Recibido: %d", sub_rcv->puerto);
			broker_logger_info("IP Recibido: %s",
								ip);

			//ver diccionario para el proceso
			search_queue(sub_rcv);
			free(sub_rcv->ip);
			free(sub_rcv);

			break;
		}

		// From GC
		case CAUGHT_POKEMON:
		{
			broker_logger_info("Caught received");
			t_caught_pokemon *caught_rcv = utils_receive_and_deserialize(
					client_fd, protocol);
			broker_logger_info("ID correlacional: %d",
					caught_rcv->id_correlacional);
			broker_logger_info("ID mensaje: %d", caught_rcv->id_msg);
			broker_logger_info("Resultado (0/1): %d", caught_rcv->result);
			break;
		}

		default:
		break;
	}
}
}

void initialice_queue(){
	get_queue = list_create ();
	appeared_queue = list_create ();
	new_queue = list_create ();
	caught_queue = list_create ();
	catch_queue = list_create ();
	localized_queue = list_create ();
}

t_subscribe_nodo* check_already_subscribe(char *ip,uint32_t puerto,t_list *list){
	int find_subscribe(t_subscribe_nodo *nodo){
		return (strcmp(ip,nodo->ip)==0 && (puerto == nodo->puerto) );
	}

	return list_find(list,(void*)find_subscribe);
}

void add_to(t_list *list, char *ip, uint32_t puerto){
	if (check_already_subscribe(ip,puerto,list) == NULL){
		t_subscribe_nodo *nodo = malloc(sizeof(t_subscribe_nodo));
		nodo->ip = string_duplicate(ip);
		nodo->puerto = puerto;
		list_add(list,nodo);
	}
	else {
		broker_logger_info("Ya esta Subscripto");
	}
}





void search_queue(t_subscribe *unSubscribe){

	switch(unSubscribe->cola){
	 case NEW_QUEUE:{
		 broker_logger_info("Subscribido ip %s con puerto %d  a Cola NEW ",unSubscribe->ip,unSubscribe->puerto);
		 add_to(new_queue,unSubscribe->ip,unSubscribe->puerto);
		 break;
	 }
	 case CATCH_QUEUE:{
		 broker_logger_info("Subscribido ip %s con puerto %d  a Cola CATCH ",unSubscribe->ip,unSubscribe->puerto);
		 add_to(catch_queue,unSubscribe->ip,unSubscribe->puerto);
		 break;
	 }
	 case CAUGHT_QUEUE:{
		 broker_logger_info("Subscribido ip %s con puerto %d  a Cola CAUGHT ",unSubscribe->ip,unSubscribe->puerto);
		 add_to(caught_queue,unSubscribe->ip,unSubscribe->puerto);
		 break;
	 }
	 case GET_QUEUE:{
		 broker_logger_info("Subscribido ip %s con puerto %d  a Cola GET ",unSubscribe->ip,unSubscribe->puerto);
		 add_to(get_queue,unSubscribe->ip,unSubscribe->puerto);
		 break;
	 }
	 case LOCALIZED_QUEUE:{
		 broker_logger_info("Subscribido ip %s con puerto %d  a Cola LOCALIZED ",unSubscribe->ip,unSubscribe->puerto);
		 add_to(localized_queue,unSubscribe->ip,unSubscribe->puerto);
		 break;
	 }
	 case APPEARED_QUEUE:{
		 broker_logger_info("Subscribido ip %s con puerto %d  a Cola APPEARED ",unSubscribe->ip,unSubscribe->puerto);
		 add_to(appeared_queue,unSubscribe->ip,unSubscribe->puerto);
		 break;
	 }

	}
}

void broker_exit() {
	socket_close_conection(broker_socket);
	broker_config_free();
	broker_logger_destroy();
}
