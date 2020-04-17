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
	CATCH_POKEMON,
	CAUGHT_POKEMON,
	GET_POKEMON,
	ID_GENERATE,
	LOCALIZED_POKEMON,
	SUBSCRIBE
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

typedef struct {
	uint32_t id_correlacional;
	char* nombre_pokemon;
	uint32_t tamanio_nombre;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t id_gen;
} t_catch_pokemon;

typedef struct {
	uint32_t id_correlacional;
	char* nombre_pokemon;
	uint32_t tamanio_nombre;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t id_generado;
} t_catch_pokemon_gc;

typedef struct {
	uint32_t id_generado;
} t_generate;

typedef struct {
	uint32_t id_correlacional;
	uint32_t id_msg;
	uint32_t result;
} t_caught_pokemon;

typedef struct {
	uint32_t id_correlacional;
	char* nombre_pokemon;
	uint32_t tamanio_nombre;
} t_get_pokemon;

typedef struct {
	char* ip;
	uint32_t puerto;
} t_subscribe;

typedef struct {
	uint32_t id_correlacional;
	char* nombre_pokemon;
	uint32_t tamanio_nombre;
	uint32_t cant_elem;
	// TODO: Lista de posiciones
} t_localized_pokemon;

#endif /* COMMON_PROTOCOLS_H_ */
