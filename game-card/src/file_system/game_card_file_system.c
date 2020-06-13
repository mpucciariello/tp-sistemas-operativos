#include "game_card_file_system.h"


void gcfsCreateStructs(){
	createRootFiles();
	setupMetadata();
	// mostrar_bitarray(bitmap);
	// game_card_logger_info("cantidad %d", getFreeBlocks(lfsMetaData.blocks, bitmap));
	
	
	//Prueba 1
	t_new_pokemon newPokemon;
	newPokemon.nombre_pokemon = "Pikachu";

	newPokemon.cantidad = 10;
	newPokemon.pos_x = 2;
	newPokemon.pos_y = 5;

	//Prueba 2
	t_new_pokemon newPokemon2;
	newPokemon2.nombre_pokemon = "Pikachu";
	
	newPokemon2.cantidad = 3;
	newPokemon2.pos_x = 1;
	newPokemon2.pos_y = 9;

	t_new_pokemon newPokemon3;
	newPokemon3.nombre_pokemon = "Charmander";
	
	newPokemon3.cantidad = 101;
	newPokemon3.pos_x = 4;
	newPokemon3.pos_y = 1;

	t_new_pokemon newPokemon4;
	newPokemon4.nombre_pokemon = "Charmander";
	
	newPokemon4.cantidad = 15;
	newPokemon4.pos_x = 517;
	newPokemon4.pos_y = 2046;

	t_new_pokemon newPokemon6;
	newPokemon6.nombre_pokemon = "Charmander";
	
	newPokemon6.cantidad = 1;
	newPokemon6.pos_x = 413;
	newPokemon6.pos_y = 17;

	t_new_pokemon newPokemon7;
	newPokemon7.nombre_pokemon = "Charmander";
	
	newPokemon7.cantidad = 29;
	newPokemon7.pos_x = 723;
	newPokemon7.pos_y = 97;

	t_new_pokemon newPokemon8;
	newPokemon8.nombre_pokemon = "Charmander";
	
	newPokemon8.cantidad = 3001;
	newPokemon8.pos_x = 65;
	newPokemon8.pos_y = 93;

	t_new_pokemon newPokemon9;
	newPokemon9.nombre_pokemon = "Charmander";
	
	newPokemon9.cantidad = 34;
	newPokemon9.pos_x = 12;
	newPokemon9.pos_y = 17;

	t_new_pokemon newPokemon10;
	newPokemon10.nombre_pokemon = "Charmander";
	
	newPokemon10.cantidad = 11;
	newPokemon10.pos_x = 129;
	newPokemon10.pos_y = 547;


	
	t_catch_pokemon catchPokemon;
	catchPokemon.nombre_pokemon = "Pikachu";
	catchPokemon.pos_x = 2;
	catchPokemon.pos_y = 5;

	catchAPokemon(catchPokemon);
	
	//createNewPokemon(newPokemon2);
	
	/*
	createNewPokemon(newPokemon3);
	createNewPokemon(newPokemon4);
	createNewPokemon(newPokemon6);
	createNewPokemon(newPokemon7);
	createNewPokemon(newPokemon8);
	createNewPokemon(newPokemon9);
	createNewPokemon(newPokemon10); */
	
	game_card_logger_info("Termino todo OK");
}

int createRecursiveDirectory(const char* path) {
	char* completePath = string_new();
	char* newDirectoryMetadata = string_new();
	char* super_path = (char*) malloc(strlen(path) +1);
	char* nombre = (char*) malloc(strlen(path)+1);

	string_append(&completePath, struct_paths[TALL_GRASS]);
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


// Add pokemon total if coordinate exists
void addTotalPokemonIfCoordinateExist(t_new_pokemon newPokemon, t_list* pokemonLines) {
	for (int i=0; i<list_size(pokemonLines); i++) {
		blockLine* pokemonLineBlock = list_get(pokemonLines, i);
		if (pokemonLineBlock->posX == newPokemon.pos_x && pokemonLineBlock->posY == newPokemon.pos_y) {
			pokemonLineBlock->cantidad = pokemonLineBlock->cantidad + newPokemon.cantidad;
		}
	}
}

// Delete pokemon total if coordinate exists
void deletePokemonTotalIfCoordinateExist(t_catch_pokemon catchPokemon, t_list* pokemonLines) {
	for (int i=0; i<list_size(pokemonLines); i++) {
		blockLine* pokemonLineBlock = list_get(pokemonLines, i);
		if (pokemonLineBlock->posX == catchPokemon.pos_x && pokemonLineBlock->posY == catchPokemon.pos_y) {
			if (pokemonLineBlock->cantidad != 1) {
				pokemonLineBlock->cantidad = pokemonLineBlock->cantidad - 1;	
			} else {
				list_remove(pokemonLines, i);
			}
		}
	}
}

// ToDO
// 1) Liberar memoria 
// 2) Esperar cantidad de segundos definidos 
// 3) Verificar si se puede abrir el archivo (si no lo esta abriendo otro) y reintentar luego de un tiempo
void createNewPokemon(t_new_pokemon newPokemon) {
	game_card_logger_info("New Pokemon: %s", newPokemon.nombre_pokemon);
	char* completePath = string_new();
	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, newPokemon.nombre_pokemon);
	int freeBlocks = getFreeBlocks(lfsMetaData.blocks, bitmap);

	// Existe Pokemon
	if (access(completePath, F_OK) != -1) {
		game_card_logger_info("Ya existe ese Pokemon. Se leen las estructuras");
		pokemonMetadata pokemonMetadata = readPokemonMetadata(completePath);

		t_list* listBlocks = stringBlocksToList(pokemonMetadata.blocks);
		t_list* pokemonLines = readPokemonLines(listBlocks);

		if (coordinateExists(newPokemon.pos_x, newPokemon.pos_y, pokemonLines) == 1) {
			addTotalPokemonIfCoordinateExist(newPokemon, pokemonLines);
		} else {
			blockLine* newNode = createBlockLine(newPokemon.pos_x, newPokemon.pos_y, newPokemon.cantidad);
			list_add(pokemonLines, newNode);
		}

		char* stringToWrite = formatListToStringLine(pokemonLines);
		int blocksRequired = cuantosBloquesOcupa(stringToWrite);
		char* stringLength = string_itoa(strlen(stringToWrite));

		if (freeBlocks > blocksRequired) {
			// Necesito pedir bloques
			if (blocksRequired > list_size(listBlocks)) {
				int extraBlocksNeeded = blocksRequired - list_size(listBlocks);
				t_list* extraBlocks = requestFreeBlocks(extraBlocksNeeded);
				// Agrego los nuevos bloques en la lista original
				list_add_all(listBlocks, extraBlocks);
			} 
∑
		} else {
			game_card_logger_error("No hay bloques disponibles. No se puede hacer la operacion");
		}

	} else {
		game_card_logger_info("No existe ese Pokemon. Se crean y escriben las estructuras.");
		char* super_path = (char*) malloc(strlen(newPokemon.nombre_pokemon) +1);
		char* pokemonDirectory = (char*) malloc(strlen(newPokemon.nombre_pokemon)+1);
	
		if (string_contains(newPokemon.nombre_pokemon, "/")) {
	    	split_path(newPokemon.nombre_pokemon, &super_path, &pokemonDirectory);
			char* filePath = string_new();
			string_append(&filePath, "Files/");
			string_append(&filePath, super_path);
			createRecursiveDirectory(filePath);
		}

		createFile(newPokemon.nombre_pokemon);

		char* pokemonPerPosition = formatToBlockLine(newPokemon.pos_x, newPokemon.pos_y, newPokemon.cantidad);
		int pokemonPerPositionLength = strlen(pokemonPerPosition);

		// Necesito 1 solo bloque
		if(lfsMetaData.blockSize >= pokemonPerPositionLength) {
		  
		  int blocksRequired = cuantosBloquesOcupa(pokemonPerPosition);

		  if (freeBlocks > blocksRequired) {
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
		  } else {
			game_card_logger_error("No hay bloques disponibles. No se puede hacer la operacion");
		  }
		} else if(lfsMetaData.blockSize < pokemonPerPositionLength) {
		  
		  t_list* pokemonLines = list_create();
		  blockLine* newNode = createBlockLine(newPokemon.pos_x, newPokemon.pos_y, newPokemon.cantidad);
		  list_add(pokemonLines, newNode);

		  char* stringToWrite = formatListToStringLine(pokemonLines);
		  int blocksRequired = cuantosBloquesOcupa(stringToWrite);

		  if (freeBlocks > blocksRequired) {
			char* stringLength = string_itoa(strlen(stringToWrite));
			t_list* listBlocks = requestFreeBlocks(blocksRequired);
			writeBlocks(stringToWrite, listBlocks);
			char* metadataBlocks = formatToMetadataBlocks(listBlocks);
			updatePokemonMetadata(newPokemon.nombre_pokemon, "N", stringLength, metadataBlocks, "Y");
		  } else {
			  game_card_logger_error("No hay bloques disponibles. No se puede hacer la operacion");
		  }
		  
	  	}
	}
	
}

t_list* requestFreeBlocks(int extraBlocksNeeded) {
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


int catchAPokemon(t_catch_pokemon catchPokemon) {
	game_card_logger_info("Catch Pokemon: %s", catchPokemon.nombre_pokemon);
	char* completePath = string_new();
	int res = 0;
	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, catchPokemon.nombre_pokemon);

	if (access(completePath, F_OK) != -1) {
		game_card_logger_info("Existe el pokemon, se leen las estructuras");
		pokemonMetadata pokemonMetadata = readPokemonMetadata(completePath);
		t_list* listBlocks = stringBlocksToList(pokemonMetadata.blocks);
		t_list* pokemonLines = readPokemonLines(listBlocks);

		if (coordinateExists(catchPokemon.pos_x, catchPokemon.pos_y, pokemonLines) == 1) {
			deletePokemonTotalIfCoordinateExist(catchPokemon, pokemonLines);
			char* stringToWrite = formatListToStringLine(pokemonLines);
			int blocksRequired = cuantosBloquesOcupa(stringToWrite);
			char* stringLength = string_itoa(strlen(stringToWrite));

			if (blocksRequired == list_size(listBlocks)) {
				writeBlocks(stringToWrite, listBlocks);
				char* metadataBlocks = formatToMetadataBlocks(listBlocks);
				updatePokemonMetadata(catchPokemon.nombre_pokemon, "N", stringLength, metadataBlocks, "Y");
			}

			if (blocksRequired < list_size(listBlocks)) {
				// libero un bloque en el bitmap
				// dejo limpio el ultimo bloque
			}
		} else {
			game_card_logger_error("No existen las coordenadas para ese pokemon, no se puede completar la operacion.");
		}

	} else {
		game_card_logger_error("No existe ese Pokemon en el filesystem.");
	}
}

