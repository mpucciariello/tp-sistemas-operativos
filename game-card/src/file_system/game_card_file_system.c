#include "game_card_file_system.h"


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

		//printListOfPokemonReadedLines(pokemonLines, blocks);

		if (coordinateExists(newPokemon.pos_x, newPokemon.pos_y, pokemonLines) == 1) {
			operatePokemonLine(newPokemon, pokemonLines, "+");
			char* stringToWrite = formatListToStringLine(pokemonLines);
			
			if (!stringFitsInBlocks(stringToWrite, listBlocks)) {

			} else {

			}
		}

		config_destroy(metadataFile);
	} else {
		game_card_logger_info("No existe ese Pokemon. Se crean y escriben las estructuras.");
		
		createRecursiveDirectory(super_path);
		createFile(newPokemon.nombre_pokemon);

		char* pokemonPerPosition = formatToBlockLine(newPokemon.pos_x, newPokemon.pos_y, newPokemon.cantidad);
		int pokemonPerPositionLength = strlen(pokemonPerPosition);

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
