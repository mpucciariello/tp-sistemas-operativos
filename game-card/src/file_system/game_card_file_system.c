#include "game_card_file_system.h"

static int _mkpath(char* file_path, mode_t mode)
{
	assert(file_path && *file_path);
	char* p;
	for(p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/'))
	{
		*p = '\0';
		if(mkdir(file_path, mode) == -1)
		{
			if (errno != EEXIST)
			{
				*p = '/';
				return -1;
			}
		}
		*p = '/';
	}
	return 0;
}

void gcfsCreateStructs(){
	createRootFiles();
	setupMetadata();
	setupFilesDirectory();

	t_new_pokemon newPokemon;
	newPokemon.nombre_pokemon = "Pokemon/Electrico/Zapdos";
	newPokemon.cantidad = 100;
	newPokemon.pos_x = 1;
	newPokemon.pos_y = 1;
	createNewPokemon(newPokemon);
	game_card_logger_info("Termino todo OK");
}

void createRootFiles() {
	char* dir_metadata = string_new();
	string_append(&dir_metadata, game_card_config->punto_montaje_tallgrass);
	string_append(&dir_metadata, "Metadata/");
	
	char* archivos = string_new();
	string_append(&archivos, game_card_config->punto_montaje_tallgrass);
	string_append(&archivos, "Files/");

	char* dir_bloques = string_new();
	string_append(&dir_bloques, game_card_config->punto_montaje_tallgrass);
	string_append(&dir_bloques, "Bloques/");

	if(_mkpath(game_card_config->punto_montaje_tallgrass, 0755) == -1) {
		game_card_logger_error("_mkpath");
	} else {
		// Creo carpetas
		mkdir(dir_metadata, 0777);
		game_card_logger_info("Creada carpeta Metadata/");
		mkdir(archivos, 0777);
		game_card_logger_info("Creada carpeta Files/");
		game_card_logger_info("Creada carpeta Files/ %s", dir_bloques);
		mkdir(dir_bloques, 0777);
		game_card_logger_info("Creada carpeta Bloques/");
	}

	struct_paths[METADATA] = dir_metadata;
	struct_paths[FILES] = archivos;
	struct_paths[BLOCKS] = dir_bloques;
}

void setupMetadata() {
	char* metadataBin = string_new();
	char* bitmapBin = string_new();

	string_append(&metadataBin, struct_paths[METADATA]);
	string_append(&metadataBin, "Metadata.bin");

	if(access(metadataBin, F_OK) != -1) {
        readMetaData(metadataBin);
    } else {
        createMetaDataFile(metadataBin);
        readMetaData(metadataBin);
    }
	
	string_append(&bitmapBin, struct_paths[METADATA]);
	string_append(&bitmapBin, "Bitmap.bin");

	if(access(bitmapBin, F_OK) != -1){
        readBitmap(bitmapBin);
    } else {
        // Creo bloques + bitmap
        createBitmap(bitmapBin);
        readBitmap(bitmapBin);
        createBlocks();
    }

	free(metadataBin);
	free(bitmapBin);
}


void setupFilesDirectory() {
	char* pokemonBasePath = string_new();
	char* pokemonBaseBin = string_new();
	string_append(&pokemonBasePath, struct_paths[FILES]);
	string_append(&pokemonBasePath, "Pokemon/");

	struct_paths[POKEMON] = pokemonBasePath;	

	mkdir(pokemonBasePath, 0777);
	
	string_append(&pokemonBaseBin, pokemonBasePath);
	string_append(&pokemonBaseBin, "Metadata.bin");

	FILE* pokemonMetadata = fopen(pokemonBaseBin, "w+b");
	t_config* pokemonConfigMetadata = config_create(pokemonBaseBin);
	config_set_value(pokemonConfigMetadata, "DIRECTORY", "Y");
	config_save(pokemonConfigMetadata);
	game_card_logger_info("Creado directorio base /Pokemon y su Metadata.bin");
	fclose(pokemonMetadata);
}

void createBlocks(){
	game_card_logger_info("Creando bloques en el path /Bloques");
	FILE * newBloque;
	for(int i=0; i <= lfsMetaData.blocks-1; i++){
        char* nroBloque = string_new();
        string_append(&nroBloque, struct_paths[BLOCKS]);
        string_append(&nroBloque, string_itoa(i));
        string_append(&nroBloque, ".bin");
        newBloque = fopen(nroBloque,"w+b");
        fclose(newBloque);
        free(nroBloque);
    }
}

void createBitmap(char* bitmapBin) {
	game_card_logger_info("Creando el Bitmap.bin por primera vez");
	bitmap_file = fopen(bitmapBin, "wb+");
	char* bitarray_limpio_temp = calloc(1, ceiling(lfsMetaData.blocks, 8));
	fwrite((void*) bitarray_limpio_temp, ceiling(lfsMetaData.blocks, 8), 1, bitmap_file);
	fflush(bitmap_file);
	free(bitarray_limpio_temp);
}

void createMetaDataFile(char* metadataBin){
	game_card_logger_info("Creando Metadata.bin por primera vez");
	FILE* metadata = fopen(metadataBin, "w+b");
	config_metadata = config_create(metadataBin);
	config_set_value(config_metadata, "BLOCK_SIZE", "64");
	config_set_value(config_metadata, "BLOCKS", "15"); // asi no tengo 5492 bloques :P
	config_set_value(config_metadata, "MAGIC_NUMBER", "TALL_GRASS");
	config_save(config_metadata);
	fclose(metadata);
}

void readMetaData(char* metadataPath) {
	game_card_logger_info("Leyendo Metadata.bin");
	t_config* metadataFile = config_create(metadataPath);
	lfsMetaData.blocks = config_get_int_value(metadataFile,"BLOCKS");
    lfsMetaData.magicNumber = string_duplicate(config_get_string_value(metadataFile,"MAGIC_NUMBER"));
	lfsMetaData.blockSize = config_get_int_value(metadataFile,"BLOCK_SIZE");
	config_destroy(metadataFile);
}

char* obtenerPathDelNumeroDeBloque(int numeroDeBloque){
	char* path_del_bloque = malloc(strlen(game_card_config->punto_montaje_tallgrass)+strlen("/Bloques")+20);
	sprintf(path_del_bloque,"%sBloques/%d.bin",game_card_config->punto_montaje_tallgrass, numeroDeBloque);
	return path_del_bloque;
}

void readBitmap(char* bitmapBin) {
	bitmap_file = fopen(bitmapBin, "rb+");
	fseek(bitmap_file, 0, SEEK_END);
	int file_size = ftell(bitmap_file);
	fseek(bitmap_file, 0, SEEK_SET);
	char* bitarray_str = (char*) mmap(NULL, file_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fileno(bitmap_file), 0);
	if(bitarray_str == (char*) -1)
	{
		game_card_logger_error("Fallo el mmap: %s", strerror(errno));
	}
	fread((void*) bitarray_str, sizeof(char), file_size, bitmap_file);
	bitmap = bitarray_create_with_mode(bitarray_str, file_size, MSB_FIRST);
}


int createRecursiveDirectory(const char* path) {
	char* completePath = string_new();
	char* newDirectoryMetadata = string_new();
	char* super_path = (char*) malloc(strlen(path) +1);
	char* nombre = (char*) malloc(strlen(path)+1);

	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, path);

	if(access(completePath, F_OK) != -1) {
        game_card_logger_info("Existe el path %s", completePath);
		return -1;
    } else {
        game_card_logger_info("No existe el path %s", completePath);
		split_path(path, &super_path, &nombre);
		
		createRecursiveDirectory(super_path);

		string_append(&newDirectoryMetadata, completePath);
		string_append(&newDirectoryMetadata, "Metadata.bin");

		mkdir(completePath, 0777);
		FILE* metadata = fopen(newDirectoryMetadata, "w+b");
		config_metadata = config_create(newDirectoryMetadata);
		config_set_value(config_metadata, "DIRECTORY", "Y");
		config_save(config_metadata);
		config_destroy(config_metadata);
		fclose(metadata);
		return 0;
    };

	free(completePath);
	free(super_path);
	free(nombre);
	return 0;
}

int createFile(const char* fullPath) {
	char* completePath = string_new();
	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, fullPath);

	if(access(completePath, F_OK) != -1) {
        game_card_logger_info("Existe el directory para ese pokemon %s", completePath);
		return -1;
    } else {
		mkdir(completePath, 0777);
		updatePokemonMetadata(fullPath, "N", "0", "[]", "Y");
	}
}

void updatePokemonMetadata(const char* fullPath, const char* directory, const char* size, const char* blocks, const char* open) {
	char* completePath = string_new();
	char* newDirectoryMetadata = string_new();
	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, fullPath);

	string_append(&newDirectoryMetadata, completePath);
	string_append(&newDirectoryMetadata, "/Metadata.bin");
	
	FILE* metadata = fopen(newDirectoryMetadata, "w+b");
	config_metadata = config_create(newDirectoryMetadata);
	config_set_value(config_metadata, "DIRECTORY", directory);
	config_set_value(config_metadata, "SIZE", size);
	config_set_value(config_metadata, "BLOCKS", blocks);
	config_set_value(config_metadata, "OPEN", open);
	config_save(config_metadata);
	config_destroy(config_metadata);
	fclose(metadata);
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

int coordinateExists(unsigned int posX, unsigned int posY, t_list* pokemonLines) {
	int coordinateExist = 0;

	for (int i=0; i<list_size(pokemonLines); i++) {
		blockLine* newLineBlock = list_get(pokemonLines, i);

		if (newLineBlock->posX == posX && newLineBlock->posY == posY) {
			coordinateExist = 1;
		}
	}
	
	return coordinateExist;
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
// Add or substract if coordinate exist
void operatePokemonLine(t_new_pokemon newPokemon, t_list* pokemonLines, char* operation) {
	for (int i=0; i<list_size(pokemonLines); i++) {
		blockLine* newLineBlock = list_get(pokemonLines, i);
		if (newLineBlock->posX == newPokemon.pos_x && newLineBlock->posY == newPokemon.pos_y) {
			if (string_contains(operation, "+")) {
				newLineBlock->cantidad = newLineBlock->cantidad + newPokemon.cantidad;
			}
			if (string_contains(operation, "-")) {
				newLineBlock->cantidad = newLineBlock->cantidad - newPokemon.cantidad;
			}
		}
	}
}

// Formatea unas coordenadas y cantidad a "1-1=100" string
char* formatToBlockLine(int intPosX, int intPosY, int intCantidad) {
	char* posX = string_itoa(intPosX);
	char* posY = string_itoa(intPosY);
	char* cantidad = string_itoa(intCantidad);
	
	char* pokemonPerPosition = string_new();
	string_append(&pokemonPerPosition, posX);
	string_append(&pokemonPerPosition, "-");
	string_append(&pokemonPerPosition, posY);
	string_append(&pokemonPerPosition, "=");
	string_append(&pokemonPerPosition, cantidad);
	string_append(&pokemonPerPosition, "\n");

	return pokemonPerPosition;
}

/*
// Formatea una lista de blockLines al string final que se va escribir "1-3=10\n1-3=50\n"
char* formatListToWriteBlock(t_list* pokemonLines) {
	
	char* retChar = string_new();
	for(int j=0; j<list_size(pokemonLines); j++) {
		blockLine* newLineBlock = list_get(pokemonLines, j);
		char* posX = string_itoa(newPokemon->pos_x);
		char* posY = string_itoa(newPokemon->pos_y);
		char* cantidad = string_itoa(newPokemon->cantidad);
		string_append(&retChar, posX);
		string_append(&retChar, "-");
		string_append(&retChar, posY);
		string_append(&retChar, "=");
		string_append(&retChar, cantidad);
		string_append(&retChar, "\n");
	}

	return retChar;
}

*/
/*
void writeBlocks(t_list* pokemonLines, t_list* listBlocks, int blocksSize, int coordinatesExist) {
	
	FILE* fileToWrite;
	int readNextBlock = 0;
	//fileToWrite = fopen(newDirectoryMetadata, "wb");

	if (coordinateExists == 1) {
		for(int i=0; i<list_size(listBlocks) && readNextBlock != 1; i ++) {
			char* blockPath = string_new();
			string_append(&blockPath, struct_paths[BLOCKS]);
			string_append(&blockPath, string_itoa(list_get(listBlocks, i)));
			string_append(&blockPath, ".bin");

			fileToWrite = fopen(blockPath, "wb");

			int sumTotalBlock = 0;
			char* stringToWrite = formatListToWriteBlock(pokemonLines);
			int pokemonPerPositionLength = strlen(pokemonPerPosition);
			fwrite(pokemonPerPosition, 1 , pokemonPerPositionLength, fileToWrite);
		}

	}

	fclose(blockFile);

}
*/

void createNewPokemon(t_new_pokemon newPokemon) {
	char* super_path = (char*) malloc(strlen(newPokemon.nombre_pokemon) +1);
	char* pokemonDirectory = (char*) malloc(strlen(newPokemon.nombre_pokemon)+1);
	split_path(newPokemon.nombre_pokemon, &super_path, &pokemonDirectory);
	char* completePath = string_new();
	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, newPokemon.nombre_pokemon);

	// Existe Pokemon
	if (access(completePath, F_OK) != -1) {
		game_card_logger_info("Ya existe ese Pokemon. Se leen las estructuras");
		char* existingPokemonMetadata = string_new();
		char* existingPokemonBlocks = string_new();
		char* blocks = string_new();
		char* isOpen = string_new();
		
		string_append(&existingPokemonMetadata, completePath);
		string_append(&existingPokemonMetadata, "/Metadata.bin");
		t_config* metadataFile = config_create(existingPokemonMetadata);
		int blockSize = config_get_int_value(metadataFile, "SIZE");
	    blocks = string_duplicate(config_get_string_value(metadataFile, "BLOCKS"));
	    isOpen = string_duplicate(config_get_string_value(metadataFile, "OPEN"));

		t_list* listBlocks = stringBlocksToList(blocks);
		
		t_list* pokemonLines = readPokemonLines(listBlocks);

		printListOfPokemonReadedLines(pokemonLines, blocks);

		if (coordinateExists(newPokemon.pos_x, newPokemon.pos_y, pokemonLines) == 1) {
			operatePokemonLine(newPokemon, pokemonLines, "+");
		}

		config_destroy(metadataFile);
	} else {
		game_card_logger_info("No existe ese Pokemon. Se crean y escriben las estructuras.");
		
		createRecursiveDirectory(super_path);
		createFile(newPokemon.nombre_pokemon);

		char* pokemonPerPosition = formatToBlockLine(newPokemon.pos_x, newPokemon.pos_y, newPokemon.cantidad);
		int pokemonPerPositionLength = strlen(pokemonPerPosition);
		game_card_logger_info("Pokemon per position %s", pokemonPerPosition);

		if(lfsMetaData.blockSize >= pokemonPerPositionLength) {
		  // Pido un bloque
		  int freeBlockPosition = getAndSetFreeBlock(bitmap, lfsMetaData.blocks);
		  char* usedBlocks = string_new();
		  string_append(&usedBlocks, "[");
		  string_append(&usedBlocks, string_itoa(freeBlockPosition));
		  string_append(&usedBlocks, "]");
		  // Restamos el /n
		  char* stringToSaveLength = string_itoa(strlen(pokemonPerPosition) - 1);

		  char* pathBloque = obtenerPathDelNumeroDeBloque(freeBlockPosition);
		  FILE* blockFile = fopen(pathBloque,"wr");
		  fwrite(pokemonPerPosition, 1 , pokemonPerPositionLength, blockFile);
		  fclose(blockFile);

		  updatePokemonMetadata(newPokemon.nombre_pokemon, "N", stringToSaveLength, usedBlocks, "Y");
		} else if(lfsMetaData.blockSize < pokemonPerPositionLength) {
		  // Pido dos bloques
		  game_card_logger_info("Proximo free block %d", getAndSetFreeBlock(bitmap, lfsMetaData.blocks));
		  game_card_logger_info("Proximo 2 free block %d", getAndSetFreeBlock(bitmap, lfsMetaData.blocks));
	  	}
	}
	

	//mostrar_bitarray(bitmap);
	
}

void gcfsFreeBitmaps() {
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);
}
