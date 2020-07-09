#ifndef BROKER_H_
#define BROKER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include "config/broker_config.h"
#include "logger/broker_logger.h"
#include "../../shared-common/common/sockets.h"
#include "../../shared-common/common/utils.h"

int broker_socket;
int uid_subscribe = 0;
time_t base_time;

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
t_list *list_msg_subscribers;

typedef struct {
	int id;
	t_cola cola;
	t_list *list;
} t_subscribe_message_node;


typedef struct {
	char* ip;
	uint32_t puerto;
	uint32_t id;
	int32_t endtime;
	uint32_t f_desc;
} t_subscribe_nodo;

typedef struct {
	t_subscribe_nodo* subscribe;
	bool ack;
} t_subscribe_ack_node;

typedef struct {
	int pointer;
	int size;
	t_cola cola;
	int id;
	time_t timestamp;
} t_nodo_memory;

typedef struct {
	void* message;
	uint32_t size_message;
} t_message_to_void;

struct buddy;
struct buddy* buddy = NULL;

struct buddy *buddy_new(int num_of_fragments);
int buddy_alloc(struct buddy *self, uint32_t size);
void buddy_free(struct buddy *self, int offset);
t_subscribe_nodo* check_already_subscribed(char *ip,uint32_t puerto,t_list *list);
t_message_to_void *convert_to_void(t_protocol protocol, void *package_recv);
void *get_from_memory(t_protocol protocol, int posicion, void *message);
int save_on_memory(t_message_to_void *message_void);
void save_node_list_memory(int pointer, int size,t_cola cola,int id);
void send_message_to_queue(t_subscribe *subscriber,t_protocol protocol);
void purge_msg();
int generar_id();
_Bool is_buddy();
void create_message_ack(int id,t_list *cola,t_cola unCola);
#endif  /* BROKER_H_ */
