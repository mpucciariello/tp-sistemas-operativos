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

void createNewPokemon(t_new_pokemon newPokemon);
int coordinateExists(unsigned int posX, unsigned int posY, t_list* pokemonLines);
void operatePokemonLine(t_new_pokemon newPokemon, t_list* pokemonLines, char* operation);
t_list* retrieveFreeBlocks(int extraBlocksNeeded);


void gcfsCreateStructs();
void gcfsFreeBitmaps();

#endif /* FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_ */
