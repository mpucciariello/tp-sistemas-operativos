#ifndef CUSTOM_UTILITARIA_H_
#define CUSTOM_UTILITARIA_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "sockets.h"
#include "serializer.h"

#define max(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define min(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })

#define ceiling(x,y) (((x) + (y) - 1) / (y))

void utils_end_string(char *string);
bool utils_is_empty(char* string);
char* utils_get_parameter_i(char** array, int i);
char* utils_get_extension(char* file_name);
char* utils_get_file_name(char* path);
bool utils_is_number(char* string);
void utils_free_array(char** array);
char* utils_array_to_string(char** array);
void utils_delay(int seconds);
void utils_buffer_create(t_package* package);
t_package* utils_package_create(t_protocol code);
void utils_package_add(t_package* package, void* value, int size);
void utils_package_destroy(t_package* package);
void utils_package_send_to(t_package* t_package, int client_socket);
void utils_serialize_and_send(int socket, int package_type, void* package);
int utils_get_buffer_size(t_list *list, int index);
void* utils_receive_and_deserialize(int socket, int package_type);
t_list* utils_receive_package(int socket_cliente);
void* utils_receive_buffer(int* size, int socket_cliente);
void utils_get_from_list_to(void *parameter,t_list *list,int index);
void utils_get_from_list_to2(void *parameter,t_list *list,int index);
void utils_get_from_list_to_malloc(void *parameter,t_list *list,int index);
void utils_destroy_list(t_buffer *self);

#endif /* CUSTOM_UTILITARIA_H_ */
