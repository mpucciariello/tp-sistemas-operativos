#ifndef GAME_CARD_HANDLER_H_
#define GAME_CARD_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "../logger/game_card_logger.h"
#include "../config/game_card_config.h"

typedef struct {
	int blockSize;
	char* blocks;
	char* isOpen;
} pokemonMetadata;

int lastchar(const char* str, char chr);
int split_path(const char* path, char** super_path, char** name);
int _mkpath(char* file_path, mode_t mode);
char* obtenerPathDelNumeroDeBloque(int numeroDeBloque);
pokemonMetadata readPokemonMetadata(char* pokemonPath);

#endif /* GAME_CARD_HANDLER_H_ */
