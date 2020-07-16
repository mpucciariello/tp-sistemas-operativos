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
time_t base_time;

int broker_load();
void broker_server_init();
static void *handle_connection(void *arg);
void broker_exit();
void search_queue(t_subscribe *unSubscribe);
void initialize_queue();
void add_to(t_list *list, t_subscribe* sub);
void compactacion();

pthread_mutex_t mpointer, mid, msubs, msave, mget, mappeared, mloc, mcatch, mcaught, mnew, mmem;


char *memory;

uint32_t pointer;

uint32_t id;

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
	int32_t endtime;
	int32_t f_desc;
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
	bool libre;
	uint32_t time_lru;
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
char* get_queue_name(t_cola q);
t_subscribe_nodo* check_already_subscribed(char *ip,uint32_t puerto,t_list *list);
t_message_to_void *convert_to_void(t_protocol protocol, void *package_recv);
void *get_from_memory(t_protocol protocol, int posicion, void *message);
int save_on_memory(t_message_to_void *message_void);
char* get_protocol_name(t_cola q);
int save_on_memory_pd(t_message_to_void *message_void,t_cola cola,int id);
void save_node_list_memory(int pointer, int size,t_cola cola,int id);
void send_all_messages(t_subscribe *subscriber);
void purge_msg();
int generar_id();
void handle_disconnection(int fdesc);
void dump();
_Bool is_buddy();
void create_message_ack(int id,t_list *cola,t_cola unCola);
int libre_nodo_memoria_first(int id_correlacional,t_cola cola,t_message_to_void *message_void);
int libre_nodo_memoria_best(int id_correlacional,t_cola cola,t_message_to_void *message_void);
void aplicar_algoritmo_reemplazo_LRU();
void aplicar_algoritmo_reemplazo_FIFO();
void estado_memoria(t_list *list);
_Bool is_buddy();
int save_on_memory_partition(t_message_to_void *message_void,t_cola cola,int id_correlacional);
#endif  /* BROKER_H_ */
