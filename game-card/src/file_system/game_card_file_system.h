#ifndef FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_
#define FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <commons/txt.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

#include "../logger/game_card_logger.h"
#include "../config/game_card_config.h"
#include "../../../shared-common/common/utils.h"


/**
 * Bloques Handler
 * */
typedef struct {
	uint32_t cantidad;
	uint32_t posX;
	uint32_t posY;
} blockLine;

int calcualarBloques(int tamanio);
int cuantosBloquesOcupa(char* value);
blockLine* createBlockLine(int intPosX, int intPosY, int intCantidad);
// Formatters
char* formatListToStringLine(t_list* pokemonLines);
t_list* stringBlocksToList(char* blocks);
blockLine* formatStringToBlockLine(char* blockline);
char* formatToBlockLine(int posX, int posY, int cantidad);

bool stringFitsInBlocks(char* stringToWrite, t_list* listBlocks);
void printListOfPokemonReadedLines(t_list* pokemonLines);

void writeBlocks(char* value, t_list* bloques);
t_list* readPokemonLines(t_list* blocks);

/**
 * Game card file system
 * */
t_dictionary* files_open;
pthread_mutex_t MUTEX_LISTA_ARCHIVO_ABIERTO;

typedef struct pokemon_open_tad {
    pthread_mutex_t mArchivo;
} pokemon_open_tad;


pokemon_open_tad* new_pokemon_open_tad();

char* formatToMetadataBlocks(t_list* blocks);
void updatePokemonMetadata(char* fullPath, char* directory, char* size, char* blocks, char* open, char* op);
int createRecursiveDirectory(char* path);
int createFile(char* fullPath);
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
void updateOpenFileState(char* fullPath, char* open, char* op);

void gcfsCreateStructs();
void gcfsFreeBitmaps();
void freeBlockLine(blockLine* newLineBlock);

/**
 * Game card handler
 * */
typedef struct {
	int blockSize;
	char* blocks;
	char* isOpen;
} pokemonMetadata;

int lastchar(char* str, char chr);
int split_path(char* path, char** super_path, char** name);
int _mkpath(char* file_path, mode_t mode);
char* obtenerPathDelNumeroDeBloque(int numeroDeBloque);
pokemonMetadata readPokemonMetadata(char* pokemonPath);

/**
 * Bitmap
 * */
void mostrar_bitarray(t_bitarray* bitmap);
void setear_bloque_ocupado_en_posicion(t_bitarray* bitmap, off_t pos);
void setear_bloque_libre_en_posicion(t_bitarray* bitmap, off_t pos);
bool testear_bloque_libre_en_posicion(t_bitarray* bitmap, int pos);
int getAndSetFreeBlock(t_bitarray* bitmap, unsigned int blocks);
int getFreeBlocks(int metadataBlocks, t_bitarray* bitmap);

/**
 * Setup
 * */

typedef enum {
	METADATA,
	FILES,
	BLOCKS,
	POKEMON,
	TALL_GRASS
} e_paths_structure;

typedef struct {
	unsigned int blockSize, blocks;
	char* magicNumber;
}Metadata_LFS;

Metadata_LFS lfsMetaData;

t_config* config_metadata;
t_config* config_table_metadata;
t_bitarray* bitmap;
FILE* bitmap_file;
char* struct_paths[5];


void createRootFiles();
void setupMetadata();
void setupFilesDirectory();
void createBlocks();
void createBitmap(char* bitmapBin);
void createMetaDataFile(char* metadataBin);
void readBitmap(char* bitmapBin);
void readMetaData(char* metadataPath);

#endif /* FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_ */
