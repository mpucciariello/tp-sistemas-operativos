#include "bloques_handler.h"

int cuantosBloquesOcupa(char* value) {
    int tamanio = string_length(value);
    return calcualarBloques(tamanio);
}

int calcualarBloques(int tamanio) {
    // Redondea hacia arriba
    return 1 + ((tamanio - 1) / lfsMetaData.blockSize);
}

void writeBlocks(char* value, t_list* bloques) {

    int limite = string_length(value);
    char* valorAGuardar = string_duplicate(value);

    for(int i = 0; i < list_size(bloques) && limite > 0; i++) {

        char* pathBloque = obtenerPathDelNumeroDeBloque((int) list_get(bloques, i));

        FILE * bloque = fopen(pathBloque, "w+");

        int limiteSuperior;

        if(limite >= lfsMetaData.blockSize) {
            limiteSuperior = lfsMetaData.blockSize;
        } else {
            limiteSuperior = limite;
        }

        char* take = string_substring(valorAGuardar, 0, limiteSuperior);

        int write = fwrite(take,1,limiteSuperior,bloque);
         limite -= string_length(take);

        if(limite > 0) {
            valorAGuardar = string_substring_from(valorAGuardar, limiteSuperior);
        }

        fclose(bloque);
        free(pathBloque);
        free(take);
    }

    free(valorAGuardar);
}

// Dado una lista de bloques t_list 1,2,3 se leen los contenidos de dichos bloques
// y se retorna una lista con los contenidos leidos 
// t_list(int) => t_list (blockLine)
// is_break determina si el contenido fue partido en otro bloque (si es que al grabar no entro todo el contenido y el \n esta en el bloque siguiente)
t_list* readPokemonLines(t_list* blocks) {
	t_list* retList = list_create();
	size_t len = 0;
	char* line = NULL;
	ssize_t read;
	FILE* blockFile;
	int isBreakFile = 0;
	char* previousLastLine = string_new();

	for (int i = 0; i < list_size(blocks); i++) {
		char* blockPath = string_new();
        string_append(&blockPath, struct_paths[BLOCKS]);
        string_append(&blockPath, string_itoa(list_get(blocks, i)));
		string_append(&blockPath, ".bin");

		blockFile = fopen(blockPath, "r");

		if (blockFile == NULL) {
			game_card_logger_error("No ha sido posible leer el archivo");
		}

		while((read = getline(&line, &len, blockFile)) != -1) {
			blockLine* blockLine;
			if(string_contains(line, "\n") == 0) {
				isBreakFile = 1;
				string_append(&previousLastLine, line);
			} else if(isBreakFile == 1) {
				string_append(&previousLastLine, line);
				blockLine = formatStringToBlockLine(previousLastLine);
				isBreakFile = 0;
				previousLastLine = string_new();
				list_add(retList, blockLine);
			} else {
				blockLine = formatStringToBlockLine(line);
				list_add(retList, blockLine);
			}
		}
	}

	fclose(blockFile);
	if (line) free(line);


	return retList;
}


// Dado un string con formato [1,2,3,...] se devuelve una lista con los enteros que simbolizan un numero de bloque
t_list* stringBlocksToList(char* blocks) {
	t_list* retList = list_create();
	// Solo esta usando un bloque
	if (strlen(blocks) <= 3) {
		char* blockStrWithoutBraces = string_substring(blocks, 1, 1);
		list_add(retList, atoi(blockStrWithoutBraces));
	} // Mas de un bloque siendo usado
	else {
		char* blocksStrWithoutBraces = string_substring(blocks, 1, strlen(blocks) - 2);
		char** blocksWithoutCommaSeparator = string_split(blocksStrWithoutBraces, ",");
		int i = 0;
		while(blocksWithoutCommaSeparator[i] != NULL) {
			list_add(retList, atoi(blocksWithoutCommaSeparator[i]));
			i++;
		}
	}

	return retList;
}

// Dado una linea con formato "1-1=100/n" se devuelve la estructura correspondiente para poder manipularla
blockLine* formatStringToBlockLine(char* blockline) {
	blockLine* newLineBlock = malloc(sizeof(blockLine));
	char** splittedLine = string_split(blockline, "=");
	char** coordinates = string_split(splittedLine[0], "-");
	newLineBlock->posX = atoi(coordinates[0]);
	newLineBlock->posY = atoi(coordinates[1]);
	newLineBlock->cantidad = atoi(splittedLine[1]);
	return newLineBlock;
}

// Formatea una lista de blockLines al string final que se va escribir "1-3=10\n1-3=50\n"
char* formatListToStringLine(t_list* pokemonLines) {
	char* retChar = string_new();
	for(int j=0; j<list_size(pokemonLines); j++) {
		blockLine* newLineBlock = list_get(pokemonLines, j);
		string_append(&retChar, formatToBlockLine(newLineBlock->posX, newLineBlock->posY, newLineBlock->cantidad));
	}
	return retChar;
}


// Formatea unas coordenadas y cantidad a "1-1=100" string
char* formatToBlockLine(int intPosX, int intPosY, int intCantidad) {
	char* pokemonPerPosition = string_new();
	char* posX = string_itoa(intPosX);
	char* posY = string_itoa(intPosY);
	char* cantidad = string_itoa(intCantidad);
	
	string_append(&pokemonPerPosition, posX);
	string_append(&pokemonPerPosition, "-");
	string_append(&pokemonPerPosition, posY);
	string_append(&pokemonPerPosition, "=");
	string_append(&pokemonPerPosition, cantidad);
	string_append(&pokemonPerPosition, "\n");

	return pokemonPerPosition;
}

blockLine* createBlockLine(int intPosX, int intPosY, int intCantidad) {
	blockLine* newLineBlock = malloc(sizeof(blockLine));
	newLineBlock->posX = intPosX;
	newLineBlock->posY = intPosY;
	newLineBlock->cantidad = intCantidad;
	return newLineBlock;
}

void printListOfPokemonReadedLines(t_list* pokemonLines, char* blocks) {
	game_card_logger_info("Printeando lista para los bloques %s:", blocks);
	for (int i=0; i<list_size(pokemonLines); i++) {
		blockLine* newLineBlock = list_get(pokemonLines, i);
		game_card_logger_info("Size lista %d:", list_size(pokemonLines));
		game_card_logger_info("Elemento i %d:", i);
		game_card_logger_info("Pokemon Line %s:", formatToBlockLine(newLineBlock->posX, newLineBlock->posY, newLineBlock->cantidad));
	}
}

//Chequea si el string a escribir entra en los bloques
bool stringFitsInBlocks(char* stringToWrite, t_list* listBlocks) {
	int stringToWriteSize = strlen(stringToWrite);
	int spaceToAllocateString = list_size(listBlocks) * lfsMetaData.blockSize;
	return stringToWriteSize <= spaceToAllocateString;
}

