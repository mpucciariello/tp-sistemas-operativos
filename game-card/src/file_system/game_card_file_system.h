#ifndef FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_
#define FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>

#include "../logger/game_card_logger.h"
#include "../config/game_card_config.h"
#include "../shared-common/common/utils.h"


#include "./bloques_handler.h"
#include "./game_card_handler.h"

char* formatToMetadataBlocks(t_list* blocks);
void updatePokemonMetadata(const char* fullPath, const char* directory, const char* size, const char* blocks, const char* open);
int createRecursiveDirectory(const char* path);
int createFile(const char* fullPath);

void createNewPokemon(t_new_pokemon* newPokemon);
t_list* getAPokemon(t_get_pokemon* getPokemon);
int catchAPokemon(t_catch_pokemon* catchPokemon);

int coordinateExists(unsigned int posX, unsigned int posY, t_list* pokemonLines);
void addTotalPokemonIfCoordinateExist(t_new_pokemon* newPokemon, t_list* pokemonLines);
void deletePokemonTotalIfCoordinateExist(t_catch_pokemon* catchPokemon, t_list* pokemonLines);
t_list* requestFreeBlocks(int extraBlocksNeeded);

int calcualarBloques(int tamanio);
int cuantosBloquesOcupa(char* value);
char* crearPathBloque(int bloque, char* montajeBloques);

void gcfsCreateStructs();
void gcfsFreeBitmaps();

#endif /* FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_ */
