
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

typedef enum {
	METADATA,
	FILES,
	BLOCKS,
	POKEMON
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


void createRootFiles();
void setupMetadata();
void setupFilesDirectory();
void createBlocks();
void createBitmap(char* bitmapBin);
void createMetaDataFile(char* metadataBin);
void readBitmap(char* bitmapBin);
void readMetaData(char* metadataPath);
