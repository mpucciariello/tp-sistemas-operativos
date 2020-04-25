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

typedef enum
{
	METADATA, FILES, BLOCKS
} e_paths_structure;

typedef struct {
	unsigned int tamanioBloque,
	cantidadDeBloques;
	char* magicNumber;
}Metadata_LFS;

Metadata_LFS lfsmetadata;

t_config* config_metadata;
t_config* config_table_metadata;
t_bitarray* bitmap;
FILE* bitmap_file;
char* struct_paths[3];
int blocks_quantity;
int blocks_size;

void gcfs_create_structs();
void gcfs_free_bitmap();

#endif /* FILE_SYSTEM_GAME_CARD_FILE_SYSTEM_H_ */
