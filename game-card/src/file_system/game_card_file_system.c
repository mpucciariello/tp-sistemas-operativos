#include "game_card_file_system.h"


void gcfsCreateStructs(){
	createRootFiles();
	setupMetadata();
	setupFilesDirectory();

	t_new_pokemon newPokemon;
	newPokemon.nombre_pokemon = "Pokemon/Articuno";

	newPokemon.cantidad = 100;
	newPokemon.pos_x = 2;
	newPokemon.pos_y = 1;

	t_new_pokemon newPokemon2;
	newPokemon2.nombre_pokemon = "Pokemon/Articuno";
	
	newPokemon2.cantidad = 232;
	newPokemon2.pos_x = 3;
	newPokemon2.pos_y = 3;

	t_new_pokemon newPokemon3;
	newPokemon3.nombre_pokemon = "Pokemon/Articuno";
	
	newPokemon3.cantidad = 232;
	newPokemon3.pos_x = 4;
	newPokemon3.pos_y = 4;

	t_new_pokemon newPokemon4;
	newPokemon4.nombre_pokemon = "Pokemon/Articuno";
	
	newPokemon4.cantidad = 232;
	newPokemon4.pos_x = 5;
	newPokemon4.pos_y = 5;

	t_new_pokemon newPokemon6;
	newPokemon6.nombre_pokemon = "Pokemon/Articuno";
	
	newPokemon6.cantidad = 232;
	newPokemon6.pos_x = 66;
	newPokemon6.pos_y = 66;

	t_new_pokemon newPokemon7;
	newPokemon7.nombre_pokemon = "Pokemon/Articuno";
	
	newPokemon7.cantidad = 232;
	newPokemon7.pos_x = 77;
	newPokemon7.pos_y = 77;

	t_new_pokemon newPokemon8;
	newPokemon8.nombre_pokemon = "Pokemon/Articuno";
	
	newPokemon8.cantidad = 232;
	newPokemon8.pos_x = 88;
	newPokemon8.pos_y = 88;

	t_new_pokemon newPokemon9;
	newPokemon9.nombre_pokemon = "Pokemon/Articuno";
	
	newPokemon9.cantidad = 99;
	newPokemon9.pos_x = 99;
	newPokemon9.pos_y = 99;

	t_new_pokemon newPokemon10;
	newPokemon10.nombre_pokemon = "Pokemon/Articuno";
	
	newPokemon10.cantidad = 10;
	newPokemon10.pos_x = 1;
	newPokemon10.pos_y = 6;

	createNewPokemon(newPokemon);
	
	createNewPokemon(newPokemon2);
	createNewPokemon(newPokemon3);
	createNewPokemon(newPokemon4);
	createNewPokemon(newPokemon6);
	createNewPokemon(newPokemon7);
	createNewPokemon(newPokemon8);
	createNewPokemon(newPokemon9);
	createNewPokemon(newPokemon10);

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

		if (coordinateExists(newPokemon.pos_x, newPokemon.pos_y, pokemonLines) == 1) {
			operatePokemonLine(newPokemon, pokemonLines, "+");
		} else {
			blockLine* newNode = createBlockLine(newPokemon.pos_x, newPokemon.pos_y, newPokemon.cantidad);
			list_add(pokemonLines, newNode);
		}

		char* stringToWrite = formatListToStringLine(pokemonLines);
		int occupiedBlocks = cuantosBloquesOcupa(stringToWrite);
		char* stringLength = string_itoa(strlen(stringToWrite));

		// Necesito pedir bloques
		if (occupiedBlocks > list_size(listBlocks)) {
			int extraBlocksNeeded = occupiedBlocks - list_size(listBlocks);
			t_list* extraBlocks = retrieveFreeBlocks(extraBlocksNeeded);
			// Agrego los nuevos bloques en la lista original
			list_add_all(listBlocks, extraBlocks);
		} 

		writeBlocks(stringToWrite, listBlocks);
		char* metadataBlocks = formatToMetadataBlocks(listBlocks);
		updatePokemonMetadata(newPokemon.nombre_pokemon, "N", stringLength, metadataBlocks, "Y");

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
		  t_list* freeBlocks = list_create();
		  list_add(freeBlocks, freeBlockPosition);
		  char* metadataBlocks = formatToMetadataBlocks(freeBlocks);
		  char* stringLength = string_itoa(pokemonPerPositionLength);

		  char* pathBloque = obtenerPathDelNumeroDeBloque(freeBlockPosition);
		  FILE* blockFile = fopen(pathBloque,"wr");
		  fwrite(pokemonPerPosition, 1 , pokemonPerPositionLength, blockFile);
		  fclose(blockFile);

		  updatePokemonMetadata(newPokemon.nombre_pokemon, "N", stringLength, metadataBlocks, "Y");
		} else if(lfsMetaData.blockSize < pokemonPerPositionLength) {
		  
		  t_list* pokemonLines = list_create();
		  blockLine* newNode = createBlockLine(newPokemon.pos_x, newPokemon.pos_y, newPokemon.cantidad);
		  list_add(pokemonLines, newNode);

		  char* stringToWrite = formatListToStringLine(pokemonLines);
		  int occupiedBlocks = cuantosBloquesOcupa(stringToWrite);
		  char* stringLength = string_itoa(strlen(stringToWrite));

		  t_list* listBlocks = retrieveFreeBlocks(occupiedBlocks);

		  writeBlocks(stringToWrite, listBlocks);
		  char* metadataBlocks = formatToMetadataBlocks(listBlocks);
		  updatePokemonMetadata(newPokemon.nombre_pokemon, "N", stringLength, metadataBlocks, "Y");
		  
	  	}
	}
	

	//mostrar_bitarray(bitmap);
	
}

t_list* retrieveFreeBlocks(int extraBlocksNeeded) {
	t_list* retList = list_create();
	for (int i=0; i<extraBlocksNeeded; i++) {
		int freeBlockPosition = getAndSetFreeBlock(bitmap, lfsMetaData.blocks);
		list_add(retList, freeBlockPosition);
	}
	return retList;
}

// Formatea una lista de enteros a un string con formato [1, 2, 3] requerido por el Metadata
char* formatToMetadataBlocks(t_list* blocks) {
	char* retBlocks = string_new();
	string_append(&retBlocks, "[");

	if (list_size(blocks) > 1) {
		for(int i=0; i<list_size(blocks); i++) {
			string_append(&retBlocks, string_itoa(list_get(blocks, i)));
			if (i != (list_size(blocks) - 1)) string_append(&retBlocks, ",");
		}
	} else {
		string_append(&retBlocks, string_itoa(list_get(blocks, 0)));
	}

	string_append(&retBlocks, "]");
	return retBlocks;
}

void gcfsFreeBitmaps() {
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);
}
