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
#include "../shared-common/common/utils.h"

#include "./setup.h"



typedef struct {
	uint32_t cantidad;
	uint32_t posX;
	uint32_t posY;
} blockLine;

int calcualarBloques(int tamanio);
int cuantosBloquesOcupa(char* value);

// Formatters
char* formatListToStringLine(t_list* pokemonLines);
t_list* stringBlocksToList(char* blocks);
blockLine* formatStringToBlockLine(char* blockline);
char* formatToBlockLine(int posX, int posY, int cantidad);


bool stringFitsInBlocks(char* stringToWrite, t_list* listBlocks);
void printListOfPokemonReadedLines(t_list* pokemonLines, char* blocks);

void writeBlocks(char* stringToWrite, t_list* listBlocks, int blocksSize);
t_list* readPokemonLines(t_list* blocks);