#ifndef COMMON_PROTOCOLS_H_
#define COMMON_PROTOCOLS_H_

#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef enum {
	HANDSHAKE,
	MALLOC,
	MEMFREE,
	COPY,
	GET,
	MAP,
	UNMAP,
	SYNC,
	MALLOC_OK,
	FREE_RESPONSE,
	GET_OK,
	COPY_RESPONSE,
	MAP_OK,
	UNMAP_OK,
	SYNC_OK,
	SEG_FAULT,
	// SAC_CLI/SAC_SERVER PROTOCOLS
	GET_ATTR,
	UNLINK_NODE,
	MK_NODE,
	TRUNCATE,
	READ_DIR,
	READ,
	MK_DIR,
	WRITE,
	READ_RESPONSE,
	GET_ATTR_RESPONSE,
	MK_DIR_RESPONSE,
	READ_DIR_RESPONSE,
	WRITE_RESPONSE,
	RM_DIR,
	NEW_THREAD,
	THREAD_JOIN,
	THREAD_CLOSE,
	SEM_WAIT,
	SEM_SIGNAL,
} t_protocol;

typedef struct {
	t_protocol operation_code;
	t_buffer* buffer;
} t_package;

typedef struct {
	uint32_t tam;
	int id_libmuse;
} t_malloc;

typedef struct
{
	int pid;
	int tid;
} t_new_thread;

typedef struct {
	int tid;
} t_thread_join;

typedef struct {
	int tid;
} t_thread_close;

typedef struct {
	int tid;
	char* semaphore;
} t_sem_wait;

typedef struct {
	int tid;
	char* semaphore;
} t_sem_signal;

typedef struct
{
	char *path;
	size_t size;
	int flag;
	int id_libmuse;
} t_mmap;

typedef struct {
	uint32_t src;
	size_t size;
	int id_libmuse;
} t_msync;


typedef struct {
	uint32_t src;
	int id_libmuse;
	int size;
} t_get;

typedef struct {
	uint32_t dst;
	int self_id;
	int size;
	void* content;
} t_copy;

typedef struct {
	uint32_t dir;
	int self_id;
} t_free;

typedef struct {
  // TODO: Implementation
} t_map;

typedef struct {
  // TODO: Implementation
} t_unmap;

typedef struct {
	uint32_t ptr;
	size_t size;
} t_sync;

typedef struct {
	uint32_t ptr;
} t_malloc_ok;

typedef struct {
	int res;
} t_free_response;

typedef struct {
	void * res;
	int tamres;
} t_get_ok;

typedef struct {
	int res;
} t_copy_response;

typedef struct {
  // TODO: Implementation
} t_map_ok;

typedef struct {
  // TODO: Implementation
} t_unmap_ok;

typedef struct {
  // TODO: Implementation
} t_sync_ok;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_read_dir;
typedef struct {
	char* pathname;
	int id_sac_cli;
} t_get_attr;

typedef struct {
	uint32_t file_size;
	uint64_t creation_date;
	uint64_t modified_date;
	uint8_t state;
} t_get_attr_server;

typedef struct {
	char* pathname;
	int id_sac_cli;
	size_t size;
	off_t offset;
} t_read;
typedef struct {
	char* pathname;
	int id_sac_cli;
	char *buf;
	size_t size;
	off_t offset;
} t_write;

typedef struct {
	char* pathname;
	off_t new_size;
} t_truncate;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_unlink_node;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_mk_node;
typedef struct {
	char* pathname;
	int id_sac_cli;
} t_rm_directory;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_mk_directory;

typedef struct {
	t_list* nodes;
	int res;
} t_read_dir_server;

typedef struct {
	char *cli_buf;
	int response;
}t_read_server;

#endif /* COMMON_PROTOCOLS_H_ */
