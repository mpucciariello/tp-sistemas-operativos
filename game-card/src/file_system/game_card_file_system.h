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
#include "./bitmap.h"

typedef enum
{
	METADATA, FILES, BLOCKS, POKEMON
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
char* struct_paths[4];

typedef struct {
	uint32_t cantidad;
	uint32_t posX;
	uint32_t posY;
} blockLine;

void createBlocks();
void createMetaDataFile(char* metadataBin);
void createBitmap(char* bitmapBin);
void createRootFiles();
char* obtenerPathDelNumeroDeBloque(int numeroDeBloque);
void readBitmap(char* bitmapBin);
void readMetaData(char* metadataPath);

void setupFilesDirectory();
void setupMetadata();
void updatePokemonMetadata(const char* fullPath, const char* directory, const char* size, const char* blocks, const char* open);
int createRecursiveDirectory(const char* path);
int createFile(const char* fullPath);

t_list* stringBlocksToList(char* blocks);
t_list* readPokemonLines(t_list* blocks);
blockLine* formatStringToBlockLine(char* blockline);
int coordinateExists(unsigned int posX, unsigned int posY, t_list* pokemonLines);
void createNewPokemon(t_new_pokemon newPokemon);
void operatePokemonLine(t_new_pokemon newPokemon, t_list* pokemonLines, char* operation);
char* formatToBlockLine(int posX, int posY, int cantidad);


void gcfsCreateStructs();
void gcfsFreeBitmaps();

#endif /* FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_ */
