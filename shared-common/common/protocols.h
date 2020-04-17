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
	ACK,
	NEW_POKEMON,
	APPEARED_POKEMON,
} t_protocol;


typedef struct {
	t_protocol operation_code;
	t_buffer* buffer;
} t_package;


typedef struct {
	uint32_t id;
	uint32_t id_correlacional;
} t_ack;

typedef struct {
	char *pokemon;
	uint32_t largo;
	uint32_t cantidad;
	uint32_t x;
	uint32_t y;
	uint32_t id;
	uint32_t id_correlacional;
} t_new_pokemon;

typedef struct {
	char *pokemon;
	uint32_t largo;
	uint32_t cantidad;
	uint32_t x;
	uint32_t y;
	uint32_t id_correlacional;
} t_appeared_pokemon;


#endif /* COMMON_PROTOCOLS_H_ */