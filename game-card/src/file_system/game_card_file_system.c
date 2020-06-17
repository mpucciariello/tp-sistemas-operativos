#include "game_card_file_system.h"


void gcfsCreateStructs(){
	createRootFiles();
	setupMetadata();
	initSemaphore();
}

void initSemaphore() {
	pthread_mutex_init(&MUTEX_METADATA, NULL);
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
	free(newDirectoryMetadata);
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

	free(completePath);
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
	free(completePath);
	free(newDirectoryMetadata);
}

void updateOpenFileState(const char* fullPath, const char* open) {
	char* completePath = string_new();
	char* newDirectoryMetadata = string_new();
	char* blockSize = string_new();
	char* blocks = string_new();

	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, fullPath);
	string_append(&newDirectoryMetadata, completePath);
	string_append(&newDirectoryMetadata, "/Metadata.bin");


	t_config* readMetadataFile = config_create(newDirectoryMetadata);
	blockSize = string_duplicate(config_get_string_value(readMetadataFile, "SIZE"));
	blocks = string_duplicate(config_get_string_value(readMetadataFile, "BLOCKS"));
	
	FILE* metadata = fopen(newDirectoryMetadata, "w+b");
	config_metadata = config_create(newDirectoryMetadata);
	config_set_value(config_metadata, "SIZE", blockSize);
	config_set_value(config_metadata, "DIRECTORY", "N");
	config_set_value(config_metadata, "BLOCKS", blocks);
	config_set_value(config_metadata, "OPEN", open);
	config_save(config_metadata);
	

	config_destroy(config_metadata);
	fclose(metadata);
	config_destroy(readMetadataFile);
	
	free(completePath);
	free(newDirectoryMetadata);
	free(blockSize);
	free(blocks);
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
void addTotalPokemonIfCoordinateExist(t_new_pokemon* newPokemon, t_list* pokemonLines) {
	for (int i=0; i<list_size(pokemonLines); i++) {
		blockLine* pokemonLineBlock = list_get(pokemonLines, i);
		if (pokemonLineBlock->posX == newPokemon->pos_x && pokemonLineBlock->posY == newPokemon->pos_y) {
			pokemonLineBlock->cantidad = pokemonLineBlock->cantidad + newPokemon->cantidad;
		}
	}
}

// Delete pokemon total if coordinate exists
void deletePokemonTotalIfCoordinateExist(t_catch_pokemon* catchPokemon, t_list* pokemonLines) {
	for (int i=0; i<list_size(pokemonLines); i++) {
		blockLine* pokemonLineBlock = list_get(pokemonLines, i);
		if (pokemonLineBlock->posX == catchPokemon->pos_x && pokemonLineBlock->posY == catchPokemon->pos_y) {
			if (pokemonLineBlock->cantidad != 1) {
				pokemonLineBlock->cantidad = pokemonLineBlock->cantidad - 1;	
			} else {
				list_remove(pokemonLines, i);
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
	} 
	
	if (list_size(blocks) == 1) {
		string_append(&retBlocks, string_itoa(list_get(blocks, 0)));
	}

	string_append(&retBlocks, "]");
	return retBlocks;
}

void gcfsFreeBitmaps() {
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);
}


void freeBlockLine(blockLine* newLineBlock) {
	free(newLineBlock);
}


void createNewPokemon(t_new_pokemon* newPokemon) {
	game_card_logger_info("New Pokemon: %s", newPokemon->nombre_pokemon);
	char* completePath = string_new();
	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, newPokemon->nombre_pokemon);
	int freeBlocks = getFreeBlocks(lfsMetaData.blocks, bitmap);

	// Existe Pokemon
	if (access(completePath, F_OK) != -1) {
		game_card_logger_info("Pokemon existe dentro del FS!.");
		operateNewPokemonFile(newPokemon, completePath, freeBlocks);
	} else {
		game_card_logger_info("No existe ese Pokemon. Se crean y escriben las estructuras.");
		char* super_path = (char*) malloc(strlen(newPokemon->nombre_pokemon) +1);
		char* pokemonDirectory = (char*) malloc(strlen(newPokemon->nombre_pokemon)+1);
	
		if (string_contains(newPokemon->nombre_pokemon, "/")) {
	    	split_path(newPokemon->nombre_pokemon, &super_path, &pokemonDirectory);
			char* filePath = string_new();
			string_append(&filePath, "Files/");
			string_append(&filePath, super_path);
			createRecursiveDirectory(filePath);
			free(filePath);
		}

		createFile(newPokemon->nombre_pokemon);

		char* pokemonPerPosition = formatToBlockLine(newPokemon->pos_x, newPokemon->pos_y, newPokemon->cantidad);
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
			updatePokemonMetadata(newPokemon->nombre_pokemon, "N", stringLength, metadataBlocks, "N");
			game_card_logger_info("Operacion NEW_POKEMON terminada correctamente");
			
			fclose(blockFile);
			free(metadataBlocks);
			free(stringLength);
			free(pathBloque);
			list_destroy(freeBlocks);

		  } else {
			game_card_logger_error("No hay bloques disponibles. No se puede hacer la operacion");
		  }
		} else if(lfsMetaData.blockSize < pokemonPerPositionLength) {
		  
		  t_list* pokemonLines = list_create();
		  blockLine* newNode = createBlockLine(newPokemon->pos_x, newPokemon->pos_y, newPokemon->cantidad);
		  list_add(pokemonLines, newNode);

		  char* stringToWrite = formatListToStringLine(pokemonLines);
		  int blocksRequired = cuantosBloquesOcupa(stringToWrite);

		  if (freeBlocks > blocksRequired) {
			char* stringLength = string_itoa(strlen(stringToWrite));
			t_list* listBlocks = requestFreeBlocks(blocksRequired);
			writeBlocks(stringToWrite, listBlocks);
			char* metadataBlocks = formatToMetadataBlocks(listBlocks);
			updatePokemonMetadata(newPokemon->nombre_pokemon, "N", stringLength, metadataBlocks, "N");
			game_card_logger_info("Operacion NEW_POKEMON terminada correctamente");

			list_destroy(listBlocks);
			free(metadataBlocks);
		  } else {
			  game_card_logger_error("No hay bloques disponibles. No se puede hacer la operacion");
		  }
		  
		  list_destroy_and_destroy_elements(pokemonLines, freeBlockLine);
		  free(stringToWrite);
	  	}

		free(super_path);
		free(pokemonDirectory);
		free(pokemonPerPosition);
	}
	
	free(completePath);
}

int catchAPokemon(t_catch_pokemon* catchPokemon) {
	game_card_logger_info("Catch Pokemon: %s", catchPokemon->nombre_pokemon);
	char* completePath = string_new();
	int res;
	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, catchPokemon->nombre_pokemon);

	if (access(completePath, F_OK) != -1) {
		game_card_logger_info("Existe el pokemon, se leen las estructuras");
		res = operateCatchPokemonFile(catchPokemon, completePath);
	} else {
		game_card_logger_error("No existe ese Pokemon en el filesystem.");
	}


	free(completePath);

	return res;
}



t_list* getAPokemon(t_get_pokemon* getPokemon) {
	game_card_logger_info("Get Pokemon: %s", getPokemon->nombre_pokemon);
	char* completePath = string_new();
	string_append(&completePath, struct_paths[FILES]);
	string_append(&completePath, getPokemon->nombre_pokemon);
	t_list* res;

	if (access(completePath, F_OK) != -1) {
		game_card_logger_info("Existe el pokemon, se leen las estructuras");
		res = operateGetPokemonFile(getPokemon, completePath);
	} else {
		game_card_logger_error("No existe ese Pokemon en el filesystem.");
		
	}

	free(completePath);
	return res;
}

void operateNewPokemonFile(t_new_pokemon* newPokemon, char* completePath, int freeBlocks) {
	pokemonMetadata pokemonMetadata = readPokemonMetadata(completePath);

	if(string_equals_ignore_case(pokemonMetadata.isOpen, "N")) {
		game_card_logger_info("El archivo no esta abierto por ningun proceso, se procede a abrir el mismo.");
		
		pthread_mutex_lock(&MUTEX_METADATA);
		updateOpenFileState(newPokemon->nombre_pokemon, "Y");
		pthread_mutex_unlock(&MUTEX_METADATA);
		
		t_list* listBlocks = stringBlocksToList(pokemonMetadata.blocks);
		t_list* pokemonLines = readPokemonLines(listBlocks);
		if (coordinateExists(newPokemon->pos_x, newPokemon->pos_y, pokemonLines) == 1) {
			addTotalPokemonIfCoordinateExist(newPokemon, pokemonLines);
		} else {
			blockLine* newNode = createBlockLine(newPokemon->pos_x, newPokemon->pos_y, newPokemon->cantidad);
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
				list_destroy(extraBlocks);
			} 
			writeBlocks(stringToWrite, listBlocks);
			char* metadataBlocks = formatToMetadataBlocks(listBlocks);
			
			pthread_mutex_lock(&MUTEX_METADATA);
			updatePokemonMetadata(newPokemon->nombre_pokemon, "N", stringLength, metadataBlocks, "N");
			pthread_mutex_unlock(&MUTEX_METADATA);

			game_card_logger_info("Operacion NEW_POKEMON terminada correctamente");
			free(metadataBlocks);
		} else {
			game_card_logger_error("No hay bloques disponibles. No se puede hacer la operacion");
		}

		list_destroy_and_destroy_elements(pokemonLines, freeBlockLine);
		free(stringToWrite);
		free(stringLength);
	} else {
		game_card_logger_info("Archivo abierto, se procede a reintentar luego de %d segundos", game_card_config->tiempo_de_reintento_operacion);
		sleep(game_card_config->tiempo_de_reintento_operacion);
		operateNewPokemonFile(newPokemon, completePath, freeBlocks);
	}

	free(pokemonMetadata.blocks);
	free(pokemonMetadata.isOpen);
}


t_list* operateGetPokemonFile(t_get_pokemon* getPokemon, char* completePath) {
	pokemonMetadata pokemonMetadata = readPokemonMetadata(completePath);
	t_list* res;

	if (string_equals_ignore_case(pokemonMetadata.isOpen, "N")) {
		game_card_logger_info("El archivo no esta abierto por ningun proceso, se procede a abrir el mismo.");
		
		pthread_mutex_lock(&MUTEX_METADATA);
		updateOpenFileState(getPokemon->nombre_pokemon, "Y");
		pthread_mutex_unlock(&MUTEX_METADATA);

		t_list* listBlocks = stringBlocksToList(pokemonMetadata.blocks);
		res = readPokemonLines(listBlocks);
		
		pthread_mutex_lock(&MUTEX_METADATA);
		updateOpenFileState(getPokemon->nombre_pokemon, "N");
		pthread_mutex_unlock(&MUTEX_METADATA);

		game_card_logger_info("Operacion GET_POKEMON terminada correctamente");
	} else {
		game_card_logger_info("Archivo abierto, se procede a reintentar luego de %d segundos", game_card_config->tiempo_de_reintento_operacion);
		sleep(game_card_config->tiempo_de_reintento_operacion);
		operateGetPokemonFile(getPokemon, completePath);
	}

	free(pokemonMetadata.blocks);
	free(pokemonMetadata.isOpen);
	return res;
}

int operateCatchPokemonFile(t_catch_pokemon* catchPokemon, char* completePath) {
	pokemonMetadata pokemonMetadata = readPokemonMetadata(completePath);
	int res = 0;

	if (string_equals_ignore_case(pokemonMetadata.isOpen, "N")) {
		game_card_logger_info("El archivo no esta abierto por ningun proceso, se procede a abrir el mismo.");
		
		pthread_mutex_lock(&MUTEX_METADATA);
		updateOpenFileState(catchPokemon->nombre_pokemon, "Y");
		pthread_mutex_unlock(&MUTEX_METADATA);

		t_list* listBlocks = stringBlocksToList(pokemonMetadata.blocks);
		t_list* pokemonLines = readPokemonLines(listBlocks);

		if (coordinateExists(catchPokemon->pos_x, catchPokemon->pos_y, pokemonLines) == 1) {
			deletePokemonTotalIfCoordinateExist(catchPokemon, pokemonLines);
			char* stringToWrite = formatListToStringLine(pokemonLines);
			int blocksRequired = cuantosBloquesOcupa(stringToWrite);
			char* stringLength = string_itoa(strlen(stringToWrite));

			if (strlen(stringToWrite) != 0) {
				if (blocksRequired == list_size(listBlocks)) {
					writeBlocks(stringToWrite, listBlocks);
					char* metadataBlocks = formatToMetadataBlocks(listBlocks);
					
					pthread_mutex_lock(&MUTEX_METADATA);
					updatePokemonMetadata(catchPokemon->nombre_pokemon, "N", stringLength, metadataBlocks, "N");
					pthread_mutex_unlock(&MUTEX_METADATA);
					free(metadataBlocks);
				}

				if (blocksRequired < list_size(listBlocks)) {
					int lastBlockUsing = list_get(listBlocks, list_size(listBlocks) - 1 );
					list_remove(listBlocks, list_size(listBlocks) - 1);
					writeBlocks(stringToWrite, listBlocks);
					char* metadataBlocks = formatToMetadataBlocks(listBlocks);
					
					
					pthread_mutex_lock(&MUTEX_METADATA);
					updatePokemonMetadata(catchPokemon->nombre_pokemon, "N", stringLength, metadataBlocks, "N");
					pthread_mutex_unlock(&MUTEX_METADATA);
					
					// Limpio estructuras que ya no uso
					setear_bloque_libre_en_posicion(bitmap, lastBlockUsing);
					fclose(fopen(obtenerPathDelNumeroDeBloque(lastBlockUsing), "w"));
					free(metadataBlocks);
				}
			} 

			// Edge case donde el pokemon tiene una sola linea, un solo bloque asignado y la unica coordenada es == 1
			// Asumo que el bloque se queda ocupado pero con size = 0
			if (strlen(stringToWrite) == 0 && list_size(pokemonLines) == 0) {
				int blockUsed = list_get(listBlocks, 0);
				char* metadataBlocks = formatToMetadataBlocks(listBlocks);
				char* zeroLength = string_itoa(0);

				pthread_mutex_lock(&MUTEX_METADATA);
				updatePokemonMetadata(catchPokemon->nombre_pokemon, "N", zeroLength, metadataBlocks, "N");
				pthread_mutex_unlock(&MUTEX_METADATA);
					
				// Limpio estructuras que ya no uso
				fclose(fopen(obtenerPathDelNumeroDeBloque(blockUsed), "w"));
				free(zeroLength);
				free(metadataBlocks);
			}
			res = 1;
			game_card_logger_info("Operacion CATCH_POKEMON terminada correctamente");

			free(stringToWrite);
			free(stringLength);
		} else {
			game_card_logger_error("No existen las coordenadas para ese pokemon, no se puede completar la operacion.");
		}

		list_destroy_and_destroy_elements(pokemonLines, freeBlockLine);
	} else {
		game_card_logger_info("Archivo abierto, se procede a reintentar luego de %d segundos", game_card_config->tiempo_de_reintento_operacion);
		sleep(game_card_config->tiempo_de_reintento_operacion);
		operateCatchPokemonFile(catchPokemon, completePath);
	}

	free(pokemonMetadata.blocks);
	free(pokemonMetadata.isOpen);

	return res;
}