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

void utils_end_string(char *string)
{
	if ((strlen(string) > 0) && (string[strlen(string) - 1] == NEW_LINE))
		string[strlen(string) - 1] = END_LINE;
}

bool utils_is_empty(char* string)
{
	return string == NULL || string_is_empty(string);
}

char* utils_get_parameter_i(char** array, int i)
{
	return array[i] != NULL ? array[i] : EMPTY_STRING;
}

char* utils_get_extension(char* file_name)
{
	char *extension = strrchr(file_name, DOT);
	return !extension || extension == file_name ? EMPTY_STRING : extension + 1;
}

char* utils_get_file_name(char* path)
{
	char *file = strrchr(path, SLIDE);
	return !file || file == path ? EMPTY_STRING : file + 1;
}

bool utils_is_number(char* string)
{
	for (int i = 0; i < strlen(string); i++)
	{
		if (!isdigit(string[i]))
			return false;
	}
	return strlen(string) != 0;
}

void utils_free_array(char** array)
{
	unsigned int i = 0;
	for (; array[i] != NULL; i++)
	{
		free(array[i]);
	}
	free(array);
}

char* utils_array_to_string(char** array)
{
	int i = 0;
	char* aux;
	char* ret = string_new();
	string_append(&ret, OPENING_SQUARE_BRACKET);
	while (array[i] != NULL)
	{
		aux = array[i];
		string_append(&ret, aux);
		free(aux);
		if (array[i + 1] != NULL)
		{
			string_append(&ret, COMMA);
			string_append(&ret, SPACE);
		}
		i++;
	}
	string_append(&ret, CLOSING_SQUARE_BRACKET);
	return ret;
}

void utils_delay(int seconds)
{
	int millis = 1000 * seconds;
	clock_t start = clock();
	while (clock() < start + millis)
		;
}

void utils_buffer_create(t_package* package)
{
	package->buffer = malloc(sizeof(t_buffer));
	package->buffer->size = 0;
	package->buffer->stream = NULL;
}

t_package* utils_package_create(t_protocol code)
{
	t_package* package = malloc(sizeof(t_package));
	package->operation_code = code;
	utils_buffer_create(package);

	return package;
}

void utils_package_add(t_package* package, void* value, int size)
{
	package->buffer->stream = realloc(package->buffer->stream, package->buffer->size + size + sizeof(int));

	printf("SIZEEE %d", size);
	memcpy(package->buffer->stream + package->buffer->size, &size, sizeof(int));
	memcpy(package->buffer->stream + package->buffer->size + sizeof(int), value, size);

	package->buffer->size += size + sizeof(int);
	printf("BUFFER SIZEEE %d", package->buffer->size);
}

void utils_package_destroy(t_package* package)
{
	free(package->buffer->stream);
	free(package->buffer);
	free(package);
}

void utils_package_send_to(t_package* t_package, int client_socket)
{
	int bytes = t_package->buffer->size + 2 * sizeof(int);
	void* to_send = serializer_serialize_package(t_package, bytes);
	printf("BYTES TOTAL SEND %d", bytes);
	//printf("----->bytes %d \n",bytes);
	send(client_socket, to_send, bytes, 0);

	free(to_send);
}

void utils_serialize_and_send(int socket, int protocol, void* package_send)
{
	switch (protocol)
	{
		case HANDSHAKE:
		{
			break;
		}
		case SYNC:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_msync*) package_send)->size, sizeof(size_t));
			utils_package_add(package, &((t_msync*) package_send)->src, sizeof(uint32_t));
			utils_package_add(package, &((t_msync*) package_send)->id_libmuse, sizeof(int));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case MAP:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, ((t_mmap*) package_send)->path, strlen(((t_mmap*) package_send)->path)+1);
			utils_package_add(package, &((t_mmap*) package_send)->flag, sizeof(int));
			utils_package_add(package, &((t_mmap*) package_send)->size, sizeof(size_t));
			utils_package_add(package, &((t_mmap*) package_send)->id_libmuse, sizeof(int));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case MALLOC:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_malloc*) package_send)->tam, sizeof(uint32_t));
			utils_package_add(package, &((t_malloc*) package_send)->id_libmuse, sizeof(int));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case MAP_OK:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_malloc_ok*) package_send)->ptr, sizeof(uint32_t));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case MEMFREE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_free*) package_send)->dir, sizeof(uint32_t));
			utils_package_add(package, &((t_free*) package_send)->self_id, sizeof(int));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case GET:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_get*) package_send)->src, sizeof(uint32_t));
			utils_package_add(package, &((t_get*) package_send)->id_libmuse, sizeof(int));
			utils_package_add(package, &((t_get*) package_send)->size, sizeof(int));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case COPY:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_copy*) package_send)->dst, sizeof(uint32_t));
			utils_package_add(package, &((t_copy*) package_send)->self_id, sizeof(int));
			utils_package_add(package, &((t_copy*) package_send)->size, sizeof(int));
			utils_package_add(package, ((t_copy*) package_send)->content, ((t_copy*) package_send)->size);
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case MALLOC_OK:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_malloc_ok*) package_send)->ptr, sizeof(uint32_t));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case GET_OK:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_copy*) package_send)->dst, sizeof(uint32_t));
			utils_package_add(package, &((t_copy*) package_send)->self_id, sizeof(int));
			utils_package_add(package, &((t_copy*) package_send)->size, sizeof(int));
			utils_package_add(package, ((t_copy*) package_send)->content, ((t_copy*) package_send)->size);
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case FREE_RESPONSE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_free_response*) package_send)->res, sizeof(int));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
		case READ_DIR:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_read_dir*) package_send)->id_sac_cli,sizeof(uint32_t));
			utils_package_add(package, ((t_read_dir*) package_send)->pathname, strlen(((t_read_dir*) package_send)->pathname)+1);
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case GET_ATTR:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_get_attr*) package_send)->id_sac_cli,sizeof(uint32_t));
			utils_package_add(package, ((t_get_attr*) package_send)->pathname, strlen(((t_get_attr*) package_send)->pathname)+1);
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case TRUNCATE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, ((t_truncate*) package_send)->pathname, strlen(((t_truncate*) package_send)->pathname)+1);
			utils_package_add(package, &((t_truncate*) package_send)->new_size, sizeof(off_t));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case MK_NODE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_mk_node*) package_send)->id_sac_cli,sizeof(uint32_t));
			utils_package_add(package, ((t_mk_node*) package_send)->pathname, strlen(((t_mk_node*) package_send)->pathname)+1);
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case UNLINK_NODE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_unlink_node*) package_send)->id_sac_cli,sizeof(uint32_t));
			utils_package_add(package, ((t_unlink_node*) package_send)->pathname, strlen(((t_unlink_node*) package_send)->pathname)+1);
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case RM_DIR:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_rm_directory*) package_send)->id_sac_cli,sizeof(uint32_t));
			utils_package_add(package, ((t_rm_directory*) package_send)->pathname, strlen(((t_rm_directory*) package_send)->pathname)+1);
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case MK_DIR:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_mk_directory*) package_send)->id_sac_cli,sizeof(uint32_t));
			utils_package_add(package, ((t_mk_directory*) package_send)->pathname, strlen(((t_mk_directory*) package_send)->pathname) + 1);
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case READ:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_read*) package_send)->id_sac_cli,sizeof(uint32_t));
			utils_package_add(package, ((t_read*) package_send)->pathname, strlen(((t_read*) package_send)->pathname)+1);
			utils_package_add(package, &((t_read*) package_send)->size,sizeof(size_t));
			utils_package_add(package, &((t_read*) package_send)->offset,sizeof(off_t));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case WRITE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, ((t_write*) package_send)->pathname, strlen(((t_write*) package_send)->pathname)+1);
			utils_package_add(package, &((t_write*) package_send)->id_sac_cli,sizeof(uint32_t));
			utils_package_add(package, ((t_write*) package_send)->buf, strlen(((t_write*) package_send)->buf)+1);
			utils_package_add(package, &((t_write*) package_send)->size,sizeof(size_t));
			utils_package_add(package, &((t_write*) package_send)->offset,sizeof(off_t));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case GET_ATTR_RESPONSE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_get_attr_server*) package_send)->file_size,sizeof(uint32_t));
			utils_package_add(package, &((t_get_attr_server*) package_send)->creation_date,sizeof(uint64_t));
			utils_package_add(package, &((t_get_attr_server*) package_send)->modified_date,sizeof(uint64_t));
			utils_package_add(package, &((t_get_attr_server*) package_send)->state,sizeof(uint8_t));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case READ_DIR_RESPONSE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_read_dir_server*) package_send)->res, sizeof(uint32_t));
			for (int j=0; j<list_size(((t_read_dir_server*) package_send)->nodes); j++)
			{
				char *node = list_get(((t_read_dir_server*) package_send)->nodes, j);
				utils_package_add(package, node, strlen(node) + 1);
			}
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case READ_RESPONSE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, ((t_read_server*) package_send)->cli_buf, strlen(((t_read_server*) package_send)->cli_buf)+1);
			utils_package_add(package, &((t_read_server*) package_send)->response, sizeof(uint32_t));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case NEW_THREAD:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_new_thread*) package_send)->pid,sizeof(int));
			utils_package_add(package, &((t_new_thread*) package_send)->tid,sizeof(int));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case THREAD_JOIN:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_thread_join*) package_send)->tid,sizeof(int));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case THREAD_CLOSE:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_thread_close*) package_send)->tid,sizeof(int));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case SEM_WAIT:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_sem_wait*) package_send)->tid,sizeof(int));
			utils_package_add(package, &((t_sem_wait*) package_send)->semaphore, strlen(((t_sem_wait*) package_send)->semaphore));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case SEM_SIGNAL:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_sem_signal*) package_send)->tid,sizeof(int));
			utils_package_add(package, &((t_sem_signal*) package_send)->semaphore, strlen(((t_sem_signal*) package_send)->semaphore));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}

	}
}

void* utils_receive_and_deserialize(int socket, int package_type)
{
	void iterator(t_buffer* value)
	{
		printf("%d \n", value->size);
		int dest;
		memcpy(&dest, value->stream, value->size);
		printf("%d \n", dest);
	}
	switch (package_type)
	{
		case SYNC:
		{
					t_msync *sync_request = malloc(sizeof(t_msync));
					t_list* list = utils_receive_package(socket);
					utils_get_from_list_to(&sync_request->size, list, 0);
					utils_get_from_list_to(&sync_request->src, list, 1);
					utils_get_from_list_to(&sync_request->id_libmuse, list, 2);
					list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
					return sync_request;
		}

		case MAP:
		{
				t_mmap *map_request = malloc(sizeof(t_mmap));
				t_list* list = utils_receive_package(socket);
				map_request->path = malloc(utils_get_buffer_size(list, 0));
				utils_get_from_list_to(map_request->path, list, 0);
				utils_get_from_list_to(&map_request->flag, list, 1);
				utils_get_from_list_to(&map_request->size, list, 2);
				utils_get_from_list_to(&map_request->id_libmuse, list, 3);
				list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
				return map_request;
		}
		case MAP_OK:
		{
					t_malloc_ok *map_request = malloc(sizeof(t_malloc_ok));
					t_list* list = utils_receive_package(socket);
					utils_get_from_list_to(&map_request->ptr, list, 0);
					list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
					return map_request;
		}
		case MALLOC:
		{
			t_malloc *malloc_request = malloc(sizeof(t_malloc));

			t_list* list = utils_receive_package(socket);
			//obtiene y guarda en un puntero desde un nodo de la lista dado un index
			utils_get_from_list_to(&malloc_request->tam, list, 0);
			utils_get_from_list_to(&malloc_request->id_libmuse, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return malloc_request;
		}
		case MEMFREE:
		{
			t_free *free_request = malloc(sizeof(t_free));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&free_request->dir, list, 0);
			utils_get_from_list_to(&free_request->self_id, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return free_request;
		}
		case GET:
		{
			t_get *get_request = malloc(sizeof(t_get));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->src, list, 0);
			utils_get_from_list_to(&get_request->id_libmuse, list, 1);
			utils_get_from_list_to(&get_request->size, list, 2);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case COPY:
		{
			t_copy *copy_req = malloc(sizeof(t_copy));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&copy_req->dst, list, 0);
			utils_get_from_list_to(&copy_req->self_id, list, 1);
			utils_get_from_list_to(&copy_req->size, list, 2);
			//printf("****cantidad %d***",list_size(list));
			//int size  = utils_get_buffer_size(list,3);
			//printf("****size %d***",size);
			copy_req->content = malloc(copy_req->size); ;
			utils_get_from_list_to(copy_req->content, list, 3);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);

			return copy_req;
		}
		case MALLOC_OK:
		{
			t_malloc_ok* malloc_recv = malloc(sizeof(t_malloc_ok));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&malloc_recv->ptr, list, 0);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return malloc_recv;
		}
		case FREE_RESPONSE:
		{
			t_free_response* free_recv = malloc(sizeof(t_free_response));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&free_recv->res, list, 0);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return free_recv;
		}
		case GET_OK:
		{
			t_copy *copy_req = malloc(sizeof(t_copy));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&copy_req->dst, list, 0);
			utils_get_from_list_to(&copy_req->self_id, list, 1);
			utils_get_from_list_to(&copy_req->size, list, 2);
			//printf("****cantidad %d***",list_size(list));
			//int size  = utils_get_buffer_size(list,3);
			//printf("****size %d***",size);
			copy_req->content = malloc(copy_req->size); ;
			utils_get_from_list_to(copy_req->content, list, 3);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);

			return copy_req;
		}
		case GET_ATTR:
		{
			t_get_attr *get_request = malloc(sizeof(t_get_attr));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->id_sac_cli,list,0);
			get_request->pathname = malloc(utils_get_buffer_size(list, 1));
			utils_get_from_list_to(get_request->pathname, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case READ_DIR:
		{
			t_read_dir *get_request = malloc(sizeof(t_read_dir));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->id_sac_cli,list,0);
			get_request->pathname = malloc(utils_get_buffer_size(list, 1));
			utils_get_from_list_to(get_request->pathname, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case MK_DIR:
		{
			t_mk_directory *get_request = malloc(sizeof(t_mk_directory));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->id_sac_cli,list,0);
			get_request->pathname = malloc(sizeof(get_request->pathname));
			utils_get_from_list_to(get_request->pathname, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case READ:
		{
			t_read *get_request = malloc(sizeof(t_read));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->id_sac_cli,list,0);
			get_request->pathname = malloc(utils_get_buffer_size(list, 1));
			utils_get_from_list_to(get_request->pathname, list, 1);
			utils_get_from_list_to(&get_request->size, list, 2);
			utils_get_from_list_to(&get_request->offset, list,3);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case WRITE:
		{
			t_write *get_request = malloc(sizeof(t_write));
			t_list* list = utils_receive_package(socket);
			get_request->pathname = malloc(utils_get_buffer_size(list, 0));
			utils_get_from_list_to(get_request->pathname, list, 0);
			utils_get_from_list_to(&get_request->id_sac_cli,list,1);
			get_request->buf = malloc(utils_get_buffer_size(list, 2));
			utils_get_from_list_to(get_request->buf, list, 2);
			utils_get_from_list_to(&get_request->size, list, 3);
			utils_get_from_list_to(&get_request->offset, list,4);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case TRUNCATE:
		{
			t_truncate *get_request = malloc(sizeof(t_truncate));
			t_list* list = utils_receive_package(socket);
			get_request->pathname = malloc(utils_get_buffer_size(list, 0));
			utils_get_from_list_to(get_request->pathname, list, 0);
			utils_get_from_list_to(&get_request->new_size, list,1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case UNLINK_NODE:
		{
			t_unlink_node *get_request = malloc(sizeof(t_unlink_node));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->id_sac_cli,list,0);
			get_request->pathname = malloc(utils_get_buffer_size(list, 1));
			utils_get_from_list_to(get_request->pathname, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case MK_NODE:
		{
			t_mk_node *get_request = malloc(sizeof(t_mk_node));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->id_sac_cli,list,0);
			get_request->pathname = malloc(utils_get_buffer_size(list, 1));
			utils_get_from_list_to(get_request->pathname, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case RM_DIR:
		{
			t_rm_directory *get_request = malloc(sizeof(t_rm_directory));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->id_sac_cli,list,0);
			get_request->pathname = malloc(utils_get_buffer_size(list, 1));
			utils_get_from_list_to(get_request->pathname, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case GET_ATTR_RESPONSE:
		{
			t_get_attr_server *get_request = malloc(sizeof(t_get_attr_server));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->file_size,list,0);
			utils_get_from_list_to(&get_request->creation_date,list,1);
			utils_get_from_list_to(&get_request->modified_date,list,2);
			utils_get_from_list_to(&get_request->state,list,3);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case READ_DIR_RESPONSE:
		{
			t_read_dir_server *get_request = malloc(sizeof(t_read_dir_server));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&get_request->res,list,0);
			get_request->nodes = list_create();
			for(int j=1; j<list_size(list); j++)
			{
				char *node = malloc(utils_get_buffer_size(list, j));
				utils_get_from_list_to(node,list,j);
				list_add(get_request->nodes, node);
			}
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
		case READ_RESPONSE:
		{
			t_read_server *get_request = malloc(sizeof(t_read_server));
			t_list* list = utils_receive_package(socket);
			get_request->cli_buf = malloc(utils_get_buffer_size(list, 0));
			utils_get_from_list_to(get_request->cli_buf, list, 0);
			utils_get_from_list_to(&get_request->response, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return get_request;
		}
	   case NEW_THREAD:
		{
			t_new_thread *newthread_request = malloc(sizeof(t_new_thread));
			t_list* list = utils_receive_package(socket);

			utils_get_from_list_to(&newthread_request->pid,list,0);
			utils_get_from_list_to(&newthread_request->tid,list,1);

			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return newthread_request;

		}
		case THREAD_JOIN:
		{
			t_thread_join *thread_join_request = malloc(sizeof(t_thread_join));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&thread_join_request->tid,list,0);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return thread_join_request;
		}
		case THREAD_CLOSE:
		{
			t_thread_close *thread_close_request = malloc(sizeof(t_thread_close));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&thread_close_request->tid,list,0);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return thread_close_request;
		}
		case SEM_WAIT:
		{
			t_sem_wait *sem_wait_request = malloc(sizeof(t_sem_wait));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&sem_wait_request->tid,list,0);
			utils_get_from_list_to(&sem_wait_request->semaphore, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return sem_wait_request;
		}
		case SEM_SIGNAL:
		{
			t_sem_signal *sem_signal_request = malloc(sizeof(t_sem_signal));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&sem_signal_request->tid,list,0);
			utils_get_from_list_to(&sem_signal_request->semaphore, list, 1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return sem_signal_request;
		}
	}
	return NULL;
}

void utils_destroy_list(t_buffer *self)
{
	free(self->stream);
	free(self);
}

void utils_get_from_list_to_malloc(void *parameter, t_list *list, int index)
{
	t_buffer *buffer;
	buffer = list_get(list, index);
	void *pointer = malloc(buffer->size);
	memcpy(pointer, buffer->stream, buffer->size);
	parameter =  pointer;
}

int utils_get_buffer_size(t_list *list, int index)
{
	if(list_size(list) > 0)
	{
		t_buffer *buffer;
		buffer = list_get(list, index);
		return buffer->size;
	}
	return 0;
}

void utils_get_from_list_to2(void *parameter, t_list *list, int index)
{
	t_buffer *buffer;
	buffer = list_get(list, index);
	memcpy(parameter, buffer->stream, sizeof(int));
}

void utils_get_from_list_to(void *parameter, t_list *list, int index)
{
	t_buffer *buffer;
	buffer = list_get(list, index);
	memcpy(parameter, buffer->stream, buffer->size);
}

void* utils_receive_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

t_list* utils_receive_package(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio = 0;

	buffer = utils_receive_buffer(&size, socket_cliente);

	while (desplazamiento < size)
	{
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
