#ifndef BROKER_H_
#define BROKER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "config/broker_config.h"
#include "logger/broker_logger.h"
#include "../../shared-common/common/sockets.h"
#include "../../shared-common/common/utils.h"

int broker_socket;
int uid_subscribe = 0;

int broker_load();
void broker_server_init();
static void *handle_connection(void *arg);
void broker_exit();
void search_queue(t_subscribe *unSubscribe);
void initialize_queue();
void add_to(t_list *list, t_subscribe* sub);

pthread_mutex_t mpointer,mid;

char *memory;

int pointer;

int id;
t_list *get_queue,*appeared_queue,*new_queue,*caught_queue,*catch_queue,*localized_queue;

t_list *list_memory;

typedef struct {
	char* ip;
	uint32_t puerto;
	uint32_t id;
	int32_t endtime;
	uint32_t f_desc;
} t_subscribe_nodo;

typedef struct {
	int pointer;
	int size;
	t_cola cola;
} t_nodo_memory;

typedef struct {
	int pointer;
	t_list *list;
} t_subscribe_message_node;


t_subscribe_nodo* check_already_subscribed(char *ip,uint32_t puerto,t_list *list);

typedef struct {
	void* message;
	uint32_t size_message;
} t_message_to_void;


t_message_to_void *convert_to_void(t_protocol protocol, void *package_recv);


void *get_from_memory(t_protocol protocol, int posicion, void *message);
int save_on_memory(t_message_to_void *message_void);
void save_node_list_memory(int pointer, int size,t_cola cola);
void send_message_from_queue(t_subscribe *subscriber,t_protocol protocol);
#endif /* BROKER_H_ */
