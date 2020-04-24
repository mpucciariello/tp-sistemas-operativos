#include "utils.h"

static int NEW_LINE = '\n';
static int END_LINE = '\0';
static int DOT = '.';
static int SLIDE = '/';
static char* COMMA = ",";
static char* SPACE = " ";
static char* EMPTY_STRING = "";
static char* OPENING_SQUARE_BRACKET = "[";
static char* CLOSING_SQUARE_BRACKET = "]";

void utils_end_string(char *string) {
	if ((strlen(string) > 0) && (string[strlen(string) - 1] == NEW_LINE))
		string[strlen(string) - 1] = END_LINE;
}

bool utils_is_empty(char* string) {
	return string == NULL || string_is_empty(string);
}

char* utils_get_parameter_i(char** array, int i) {
	return array[i] != NULL ? array[i] : EMPTY_STRING;
}

char* utils_get_extension(char* file_name) {
	char *extension = strrchr(file_name, DOT);
	return !extension || extension == file_name ? EMPTY_STRING : extension + 1;
}

char* utils_get_file_name(char* path) {
	char *file = strrchr(path, SLIDE);
	return !file || file == path ? EMPTY_STRING : file + 1;
}

bool utils_is_number(char* string) {
	for (int i = 0; i < strlen(string); i++) {
		if (!isdigit(string[i]))
			return false;
	}
	return strlen(string) != 0;
}

void utils_free_array(char** array) {
	unsigned int i = 0;
	for (; array[i] != NULL; i++) {
		free(array[i]);
	}
	free(array);
}

char* utils_array_to_string(char** array) {
	int i = 0;
	char* aux;
	char* ret = string_new();
	string_append(&ret, OPENING_SQUARE_BRACKET);
	while (array[i] != NULL) {
		aux = array[i];
		string_append(&ret, aux);
		free(aux);
		if (array[i + 1] != NULL) {
			string_append(&ret, COMMA);
			string_append(&ret, SPACE);
		}
		i++;
	}
	string_append(&ret, CLOSING_SQUARE_BRACKET);
	return ret;
}

void utils_delay(int seconds) {
	int millis = 1000 * seconds;
	clock_t start = clock();
	while (clock() < start + millis)
		;
}

void utils_buffer_create(t_package* package) {
	package->buffer = malloc(sizeof(t_buffer));
	package->buffer->size = 0;
	package->buffer->stream = NULL;
}

t_package* utils_package_create(t_protocol code) {
	t_package* package = malloc(sizeof(t_package));
	package->operation_code = code;
	utils_buffer_create(package);

	return package;
}

void utils_package_add(t_package* package, void* value, int size) {
	package->buffer->stream = realloc(package->buffer->stream,
			package->buffer->size + size + sizeof(int));

	memcpy(package->buffer->stream + package->buffer->size, &size, sizeof(int));
	memcpy(package->buffer->stream + package->buffer->size + sizeof(int), value,
			size);

	package->buffer->size += size + sizeof(int);
}

void utils_package_destroy(t_package* package) {
	free(package->buffer->stream);
	free(package->buffer);
	free(package);
}

void utils_package_send_to(t_package* t_package, int client_socket) {
	int bytes = t_package->buffer->size + 2 * sizeof(int);
	void* to_send = serializer_serialize_package(t_package, bytes);
	//printf("----->bytes %d \n",bytes);
	send(client_socket, to_send, bytes, 0);

	free(to_send);
}

void utils_serialize_and_send(int socket, int protocol, void* package_send) {
	switch (protocol) {

	case HANDSHAKE: {
		break;
	}

	case ACK: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_ack*) package_send)->id,
				sizeof(uint32_t));
		utils_package_add(package, &((t_ack*) package_send)->id_correlacional,
				sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}

	case NEW_POKEMON: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package,
				&((t_new_pokemon*) package_send)->tamanio_nombre,
				sizeof(uint32_t));
		utils_package_add(package,
				((t_new_pokemon*) package_send)->nombre_pokemon,
				strlen(((t_new_pokemon*) package_send)->nombre_pokemon) + 1);
		utils_package_add(package, &((t_new_pokemon*) package_send)->id,
				sizeof(uint32_t));
		utils_package_add(package,
				&((t_new_pokemon*) package_send)->id_correlacional,
				sizeof(uint32_t));
		utils_package_add(package, &((t_new_pokemon*) package_send)->cantidad,
				sizeof(uint32_t));
		utils_package_add(package, &((t_new_pokemon*) package_send)->pos_x,
				sizeof(uint32_t));
		utils_package_add(package, &((t_new_pokemon*) package_send)->pos_y,
				sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}

	case SUBSCRIBE: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, ((t_subscribe*) package_send)->ip,
				strlen(((t_subscribe*) package_send)->ip) + 1);
		utils_package_add(package, &((t_subscribe*) package_send)->puerto,
				sizeof(uint32_t));
		utils_package_add(package, &((t_subscribe*) package_send)->cola,
						sizeof(uint32_t));
		utils_package_add(package, &((t_subscribe*) package_send)->proceso,
						sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);

		break;
	}

	case CATCH_POKEMON: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package,
				&((t_catch_pokemon*) package_send)->id_correlacional,
				sizeof(uint32_t));
		utils_package_add(package,
				((t_catch_pokemon*) package_send)->nombre_pokemon,
				strlen(((t_catch_pokemon*) package_send)->nombre_pokemon) + 1);
		utils_package_add(package, &((t_catch_pokemon*) package_send)->pos_x,
				sizeof(uint32_t));
		utils_package_add(package, &((t_catch_pokemon*) package_send)->pos_y,
				sizeof(uint32_t));
		utils_package_add(package,
				&((t_catch_pokemon*) package_send)->tamanio_nombre,
				sizeof(uint32_t));
		utils_package_add(package, &((t_catch_pokemon*) package_send)->id_gen,
				sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}

	case ID_GENERATE: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_generate*) package_send)->id_generado,
				sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}

	case CAUGHT_POKEMON: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package,
				&((t_caught_pokemon*) package_send)->id_correlacional,
				sizeof(uint32_t));
		utils_package_add(package, &((t_caught_pokemon*) package_send)->id_msg,
				sizeof(uint32_t));
		utils_package_add(package, &((t_caught_pokemon*) package_send)->result,
				sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}

	case APPEARED_POKEMON: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package,
				&((t_appeared_pokemon*) package_send)->tamanio_nombre,
				sizeof(uint32_t));
		utils_package_add(package,
				((t_appeared_pokemon*) package_send)->nombre_pokemon,
				strlen(((t_new_pokemon*) package_send)->nombre_pokemon) + 1);
		utils_package_add(package,
				&((t_appeared_pokemon*) package_send)->id_correlacional,
				sizeof(uint32_t));
		utils_package_add(package, &((t_appeared_pokemon*) package_send)->pos_x,
				sizeof(uint32_t));
		utils_package_add(package, &((t_appeared_pokemon*) package_send)->pos_y,
				sizeof(uint32_t));
		utils_package_add(package,
				&((t_appeared_pokemon*) package_send)->cantidad,
				sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}

	case GET_POKEMON: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package,
				&((t_get_pokemon*) package_send)->id_correlacional,
				sizeof(uint32_t));
		utils_package_add(package,
				((t_get_pokemon*) package_send)->nombre_pokemon,
				strlen(((t_get_pokemon*) package_send)->nombre_pokemon) + 1);
		utils_package_add(package,
				&((t_get_pokemon*) package_send)->tamanio_nombre,
				sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}

	case LOCALIZED_POKEMON: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package,
				&((t_localized_pokemon*) package_send)->id_correlacional,
				sizeof(uint32_t));
		utils_package_add(package,
				((t_localized_pokemon*) package_send)->nombre_pokemon,
				strlen(((t_localized_pokemon*) package_send)->nombre_pokemon)
						+ 1);
		utils_package_add(package,
				&((t_localized_pokemon*) package_send)->tamanio_nombre,
				sizeof(uint32_t));
		utils_package_add(package,
				&((t_localized_pokemon*) package_send)->cant_elem,
				sizeof(uint32_t));
		for(int i = 0; i < ((t_localized_pokemon*) package_send)->cant_elem; i++) {
			t_position *pos = malloc(sizeof(t_position));
			pos = list_get(((t_localized_pokemon*) package_send)->posiciones, i);
			utils_package_add(package, &pos->pos_x, sizeof(int));
			utils_package_add(package, &pos->pos_y, sizeof(int));
		}
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}
	}
}

void* utils_receive_and_deserialize(int socket, int package_type) {
	void iterator(t_buffer* value) {
		printf("%d \n", value->size);
		int dest;
		memcpy(&dest, value->stream, value->size);
		printf("%d \n", dest);
	}
	switch (package_type) {

	case ACK: {
		t_ack *ack_request = malloc(sizeof(t_ack));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&ack_request->id, list, 0);
		utils_get_from_list_to(&ack_request->id_correlacional, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return ack_request;
	}

	case NEW_POKEMON: {
		t_new_pokemon *new_request = malloc(sizeof(t_new_pokemon));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&new_request->tamanio_nombre, list, 0);
		new_request->nombre_pokemon = malloc(utils_get_buffer_size(list, 1));
		utils_get_from_list_to(new_request->nombre_pokemon, list, 1);
		utils_get_from_list_to(&new_request->id, list, 2);
		utils_get_from_list_to(&new_request->id_correlacional, list, 3);
		utils_get_from_list_to(&new_request->cantidad, list, 4);
		utils_get_from_list_to(&new_request->pos_x, list, 5);
		utils_get_from_list_to(&new_request->pos_y, list, 6);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return new_request;
	}

	case APPEARED_POKEMON: {
		t_appeared_pokemon *appeared_request = malloc(
				sizeof(t_appeared_pokemon));
		t_list* list = utils_receive_package(socket);

		utils_get_from_list_to(&appeared_request->tamanio_nombre, list, 0);
		appeared_request->nombre_pokemon = malloc(
				utils_get_buffer_size(list, 1));
		utils_get_from_list_to(appeared_request->nombre_pokemon, list, 1);
		utils_get_from_list_to(&appeared_request->id_correlacional, list, 2);
		utils_get_from_list_to(&appeared_request->pos_x, list, 3);
		utils_get_from_list_to(&appeared_request->pos_y, list, 4);
		utils_get_from_list_to(&appeared_request->cantidad, list, 5);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return appeared_request;
	}

	case CATCH_POKEMON: {
		t_catch_pokemon* catch_req = malloc(sizeof(t_catch_pokemon));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&catch_req->id_correlacional, list, 0);
		catch_req->nombre_pokemon = malloc(utils_get_buffer_size(list, 1));
		utils_get_from_list_to(catch_req->nombre_pokemon, list, 1);
		utils_get_from_list_to(&catch_req->pos_x, list, 2);
		utils_get_from_list_to(&catch_req->pos_y, list, 3);
		utils_get_from_list_to(&catch_req->tamanio_nombre, list, 4);
		utils_get_from_list_to(&catch_req->id_gen, list, 5);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return catch_req;
	}

	case GET_POKEMON: {
		t_get_pokemon* get_req = malloc(sizeof(t_get_pokemon));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_req->id_correlacional, list, 0);
		get_req->nombre_pokemon = malloc(utils_get_buffer_size(list, 1));
		utils_get_from_list_to(get_req->nombre_pokemon, list, 1);
		utils_get_from_list_to(&get_req->tamanio_nombre, list, 2);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_req;
	}

	case SUBSCRIBE: {
		t_subscribe* subscribe_req = malloc(sizeof(t_generate));
		t_list* list = utils_receive_package(socket);
		subscribe_req->ip = malloc(utils_get_buffer_size(list, 0));
		utils_get_from_list_to(subscribe_req->ip, list, 0);
		utils_get_from_list_to(&subscribe_req->puerto, list, 1);
		utils_get_from_list_to(&subscribe_req->cola, list, 2);
		utils_get_from_list_to(&subscribe_req->proceso, list, 3);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return subscribe_req;
	}

	case ID_GENERATE: {
		t_generate* id_gen_req = malloc(sizeof(t_generate));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&id_gen_req->id_generado, list, 0);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return id_gen_req;
	}

	case LOCALIZED_POKEMON: {
		t_localized_pokemon* localized_req = malloc(
				sizeof(t_localized_pokemon));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&localized_req->id_correlacional, list, 0);
		localized_req->nombre_pokemon = malloc(utils_get_buffer_size(list, 1));
		utils_get_from_list_to(localized_req->nombre_pokemon, list, 1);
		utils_get_from_list_to(&localized_req->tamanio_nombre, list, 2);
		utils_get_from_list_to(&localized_req->cant_elem, list, 3);
		localized_req->posiciones = list_create();
		for(int i = 4; i < (localized_req->cant_elem*2) + 4; i+=2) {
			t_position* pos = malloc(sizeof(t_position));
			utils_get_from_list_to(&pos->pos_x, list, i);
			utils_get_from_list_to(&pos->pos_y, list, i+1);
			list_add(localized_req->posiciones, pos);
		}
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return localized_req;
	}

	case CAUGHT_POKEMON: {
		t_caught_pokemon* caught_req = malloc(sizeof(t_caught_pokemon));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&caught_req->id_correlacional, list, 0);
		utils_get_from_list_to(&caught_req->id_msg, list, 1);
		utils_get_from_list_to(&caught_req->result, list, 2);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return caught_req;
	}
	}
	return NULL;
}

void utils_destroy_list(t_buffer *self) {
	free(self->stream);
	free(self);
}

void utils_get_from_list_to_malloc(void *parameter, t_list *list, int index) {
	t_buffer *buffer;
	buffer = list_get(list, index);
	void *pointer = malloc(buffer->size);
	memcpy(pointer, buffer->stream, buffer->size);
	parameter = pointer;
}

int utils_get_buffer_size(t_list *list, int index) {
	if (list_size(list) > 0) {
		t_buffer *buffer;
		buffer = list_get(list, index);
		return buffer->size;
	}
	return 0;
}

void utils_get_from_list_to2(void *parameter, t_list *list, int index) {
	t_buffer *buffer;
	buffer = list_get(list, index);
	memcpy(parameter, buffer->stream, sizeof(int));
}

void utils_get_from_list_to(void *parameter, t_list *list, int index) {
	t_buffer *buffer;
	buffer = list_get(list, index);
	memcpy(parameter, buffer->stream, buffer->size);
}

void* utils_receive_buffer(int* size, int socket_cliente) {
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

t_list* utils_receive_package(int socket_cliente) {
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio = 0;

	buffer = utils_receive_buffer(&size, socket_cliente);

	while (desplazamiento < size) {
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		t_buffer* valor = malloc(sizeof(t_buffer));
		valor->stream = malloc(tamanio);
		valor->size = tamanio;
		memcpy(valor->stream, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}
