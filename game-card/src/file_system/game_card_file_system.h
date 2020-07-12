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
#include <commons/collections/dictionary.h>

#include "../logger/game_card_logger.h"
#include "../config/game_card_config.h"
#include "../shared-common/common/utils.h"


#include "./bloques_handler.h"
#include "./game_card_handler.h"


t_dictionary* files_open;
pthread_mutex_t MUTEX_LISTA_ARCHIVO_ABIERTO;

typedef struct pokemon_open_tad {
    pthread_mutex_t mArchivo;
} pokemon_open_tad;


pokemon_open_tad* new_pokemon_open_tad();

char* formatToMetadataBlocks(t_list* blocks);
void updatePokemonMetadata(const char* fullPath, const char* directory, const char* size, const char* blocks, const char* open);
int createRecursiveDirectory(const char* path);
int createFile(const char* fullPath);
void initSemaphore();

void createNewPokemon(t_new_pokemon* newPokemon);
t_list* getAPokemon(t_get_pokemon* getPokemon);
int catchAPokemon(t_catch_pokemon* catchPokemon);

void operateNewPokemonFile(t_new_pokemon* newPokemon, char* completePath, int freeBlocks);
t_list* operateGetPokemonFile(t_get_pokemon* getPokemon, char* completePath);
int operateCatchPokemonFile(t_catch_pokemon* catchPokemon, char* completePath);

int coordinateExists(unsigned int posX, unsigned int posY, t_list* pokemonLines);
void addTotalPokemonIfCoordinateExist(t_new_pokemon* newPokemon, t_list* pokemonLines);
void deletePokemonTotalIfCoordinateExist(t_catch_pokemon* catchPokemon, t_list* pokemonLines);
t_list* requestFreeBlocks(int extraBlocksNeeded);

int calcualarBloques(int tamanio);
int cuantosBloquesOcupa(char* value);
char* crearPathBloque(int bloque, char* montajeBloques);
void updateOpenFileState(const char* fullPath, const char* open);

void gcfsCreateStructs();
void gcfsFreeBitmaps();
void freeBlockLine(blockLine* newLineBlock);



#endif /* FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_ */
