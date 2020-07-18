#include "game_card_file_system.h"


void gcfsCreateStructs(){
	createRootFiles();
	setupMetadata();
	initSemaphore();
}

void initSemaphore() {
	pthread_mutex_init(&MUTEX_LISTA_ARCHIVO_ABIERTO, NULL);
	files_open = dictionary_create();
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
		res = list_create();
		
	}

	free(completePath);
	return res;
}

void operateNewPokemonFile(t_new_pokemon* newPokemon, char* completePath, int freeBlocks) {
	pokemonMetadata pokemonMetadata = readPokemonMetadata(completePath);

	pthread_mutex_lock (&MUTEX_LISTA_ARCHIVO_ABIERTO);
	pokemon_open_tad* pokemonOpenTad = dictionary_get(files_open, completePath);
	if (pokemonOpenTad == NULL) {
		pokemonOpenTad = new_pokemon_open_tad();
		dictionary_put(files_open, completePath, pokemonOpenTad);
	}
	pthread_mutex_unlock (&MUTEX_LISTA_ARCHIVO_ABIERTO);

	while (true) {
		if(string_equals_ignore_case(pokemonMetadata.isOpen, "N")) {
			game_card_logger_info("El archivo no esta abierto por ningun proceso, se procede a abrir el mismo.");
			
			pthread_mutex_lock(&pokemonOpenTad->mArchivo);
			updateOpenFileState(newPokemon->nombre_pokemon, "Y");
			pthread_mutex_unlock(&pokemonOpenTad->mArchivo);
			
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
				
				pthread_mutex_lock(&pokemonOpenTad->mArchivo);
				updatePokemonMetadata(newPokemon->nombre_pokemon, "N", stringLength, metadataBlocks, "N");
				pthread_mutex_unlock(&pokemonOpenTad->mArchivo);

				game_card_logger_info("Operacion NEW_POKEMON terminada correctamente");
				free(metadataBlocks);
			} else {
				game_card_logger_error("No hay bloques disponibles. No se puede hacer la operacion");
			}

			list_destroy_and_destroy_elements(pokemonLines, freeBlockLine);
			free(stringToWrite);
			free(stringLength);
			break;
		} else {
			game_card_logger_info("Archivo abierto, se procede a reintentar luego de %d segundos", game_card_config->tiempo_de_reintento_operacion);
			sleep(game_card_config->tiempo_de_reintento_operacion);
			pokemonMetadata = readPokemonMetadata(completePath);
		}
	}
	

	free(pokemonMetadata.blocks);
	free(pokemonMetadata.isOpen);
}


t_list* operateGetPokemonFile(t_get_pokemon* getPokemon, char* completePath) {
	pokemonMetadata pokemonMetadata = readPokemonMetadata(completePath);
	t_list* res;

	pthread_mutex_lock (&MUTEX_LISTA_ARCHIVO_ABIERTO);
	pokemon_open_tad* pokemonOpenTad = dictionary_get(files_open, completePath);
	if (pokemonOpenTad == NULL) {
		pokemonOpenTad = new_pokemon_open_tad();
		dictionary_put(files_open, completePath, pokemonOpenTad);
	}
	pthread_mutex_unlock (&MUTEX_LISTA_ARCHIVO_ABIERTO);

	while (true) {
		if (string_equals_ignore_case(pokemonMetadata.isOpen, "N")) {
			game_card_logger_info("El archivo no esta abierto por ningun proceso, se procede a abrir el mismo.");
			
			pthread_mutex_lock(&pokemonOpenTad->mArchivo);
			updateOpenFileState(getPokemon->nombre_pokemon, "Y");
			pthread_mutex_unlock(&pokemonOpenTad->mArchivo);

			t_list* listBlocks = stringBlocksToList(pokemonMetadata.blocks);
			res = readPokemonLines(listBlocks);
			
			pthread_mutex_lock(&pokemonOpenTad->mArchivo);
			updateOpenFileState(getPokemon->nombre_pokemon, "N");
			pthread_mutex_unlock(&pokemonOpenTad->mArchivo);

			game_card_logger_info("Operacion GET_POKEMON terminada correctamente");
			break;
		} else {
			game_card_logger_info("Archivo abierto, se procede a reintentar luego de %d segundos", game_card_config->tiempo_de_reintento_operacion);
			sleep(game_card_config->tiempo_de_reintento_operacion);
			pokemonMetadata = readPokemonMetadata(completePath);
		}
		
	}
	

	free(pokemonMetadata.blocks);
	free(pokemonMetadata.isOpen);
	return res;
}

int operateCatchPokemonFile(t_catch_pokemon* catchPokemon, char* completePath) {
	pokemonMetadata pokemonMetadata = readPokemonMetadata(completePath);
	int res = 0;

	pthread_mutex_lock (&MUTEX_LISTA_ARCHIVO_ABIERTO);
	pokemon_open_tad* pokemonOpenTad = dictionary_get(files_open, completePath);
	if (pokemonOpenTad == NULL) {
		pokemonOpenTad = new_pokemon_open_tad();
		dictionary_put(files_open, completePath, pokemonOpenTad);
	}
	pthread_mutex_unlock (&MUTEX_LISTA_ARCHIVO_ABIERTO);

	while(true) {
		if (string_equals_ignore_case(pokemonMetadata.isOpen, "N")) {
			game_card_logger_info("El archivo no esta abierto por ningun proceso, se procede a abrir el mismo.");
			
			pthread_mutex_lock(&pokemonOpenTad->mArchivo);
			updateOpenFileState(catchPokemon->nombre_pokemon, "Y");
			pthread_mutex_unlock(&pokemonOpenTad->mArchivo);

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
						
						pthread_mutex_lock(&pokemonOpenTad->mArchivo);
						updatePokemonMetadata(catchPokemon->nombre_pokemon, "N", stringLength, metadataBlocks, "N");
						pthread_mutex_unlock(&pokemonOpenTad->mArchivo);
						free(metadataBlocks);
					}

					if (blocksRequired < list_size(listBlocks)) {
						int lastBlockUsing = list_get(listBlocks, list_size(listBlocks) - 1 );
						list_remove(listBlocks, list_size(listBlocks) - 1);
						writeBlocks(stringToWrite, listBlocks);
						char* metadataBlocks = formatToMetadataBlocks(listBlocks);
						
						
						pthread_mutex_lock(&pokemonOpenTad->mArchivo);
						updatePokemonMetadata(catchPokemon->nombre_pokemon, "N", stringLength, metadataBlocks, "N");
						pthread_mutex_unlock(&pokemonOpenTad->mArchivo);
						
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

					pthread_mutex_lock(&pokemonOpenTad->mArchivo);
					updatePokemonMetadata(catchPokemon->nombre_pokemon, "N", zeroLength, metadataBlocks, "N");
					pthread_mutex_unlock(&pokemonOpenTad->mArchivo);
						
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
			break;
		} else {
			game_card_logger_info("Archivo abierto, se procede a reintentar luego de %d segundos", game_card_config->tiempo_de_reintento_operacion);
			sleep(game_card_config->tiempo_de_reintento_operacion);
			pokemonMetadata = readPokemonMetadata(completePath);
		}

	}

	free(pokemonMetadata.blocks);
	free(pokemonMetadata.isOpen);

	return res;
}


pokemon_open_tad* new_pokemon_open_tad() {
	pokemon_open_tad* pokemonOpenTad = malloc(sizeof(pokemon_open_tad));
    pthread_mutex_init(&pokemonOpenTad->mArchivo, NULL);
    return pokemonOpenTad;
}

/***
 * Game card handler
 * /

/*
 * 	Evalua si el ultimo caracter de str es chr.
 */
int lastchar(const char* str, char chr){
	if ( ( str[strlen(str)-1]  == chr) ) return 1;
	return 0;
}

/*
 * 	DESC
 * 		Divide el path con formato de [RUTA] en: [RUTA_SUPERIOR] y [NOMBRE].
 * 		Ejemplo:
 * 			path: /home/utnso/algo.txt == /home/utnso - algo.txt
 * 			path: /home/utnso/ == /home - utnso
 *
 * 	PARAM
 * 		path - Ruta a dividir
 * 		super_path - Puntero sobre el cual se guardara la ruta superior.
 * 		name - Puntero al nombre del archivo
 *
 * 	RET
 * 		0... SIEMPRE!;
 */

int split_path(const char* path, char** super_path, char** name){
	int aux;
	strcpy(*super_path, path);
	strcpy(*name, path);
	// Obtiene y acomoda el nombre del archivo.
	if (lastchar(path, '/')) {
		(*name)[strlen(*name)-1] = '\0';
	}
	*name = strrchr(*name, '/');
	*name = *name + 1; // Acomoda el nombre, ya que el primer digito siempre es '/'

	// Acomoda el super_path
	if (lastchar(*super_path, '/')) {
		(*super_path)[strlen(*super_path)-1] = '\0';
	}
	aux = strlen(*super_path) - strlen(*name);
	(*super_path)[aux] = '\0';

	return 0;
}

int _mkpath(char* file_path, mode_t mode)
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


char* obtenerPathDelNumeroDeBloque(int numeroDeBloque){
	char* path_del_bloque = malloc(strlen(game_card_config->punto_montaje_tallgrass)+strlen("/Bloques")+20);
	sprintf(path_del_bloque,"%sBloques/%d.bin",game_card_config->punto_montaje_tallgrass, numeroDeBloque);
	return path_del_bloque;
}


pokemonMetadata readPokemonMetadata(char* pokemonPath) {
	char* existingPokemonMetadata = string_new();
	char* existingPokemonBlocks = string_new();

	pokemonMetadata metadata;

	metadata.blocks = string_new();
	metadata.isOpen = string_new();

	string_append(&existingPokemonMetadata, pokemonPath);
	string_append(&existingPokemonMetadata, "/Metadata.bin");

	t_config* metadataFile = config_create(existingPokemonMetadata);
	metadata.blockSize = config_get_int_value(metadataFile, "SIZE");
	metadata.blocks = string_duplicate(config_get_string_value(metadataFile, "BLOCKS"));
	metadata.isOpen = string_duplicate(config_get_string_value(metadataFile, "OPEN"));
	config_destroy(metadataFile);

	free(existingPokemonMetadata);
	free(existingPokemonBlocks);
	return metadata;
}

/**
 * BLoques handler
 *
 * */

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

		free(blockPath);
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

	free(posX);
	free(posY);
	free(cantidad);
	return pokemonPerPosition;
}

blockLine* createBlockLine(int intPosX, int intPosY, int intCantidad) {
	blockLine* newLineBlock = malloc(sizeof(blockLine));
	newLineBlock->posX = intPosX;
	newLineBlock->posY = intPosY;
	newLineBlock->cantidad = intCantidad;
	return newLineBlock;
}

void printListOfPokemonReadedLines(t_list* pokemonLines) {
	game_card_logger_info("Size lista %d:", list_size(pokemonLines));
	for (int i=0; i<list_size(pokemonLines); i++) {
		blockLine* newLineBlock = list_get(pokemonLines, i);
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

/**
 * Bitmap
 * */
void mostrar_bitarray(t_bitarray* bitmap){
	for(int k =0;k<(bitarray_get_max_bit(bitmap));k++)
	printf("test bit posicion, es  %d en posicion %d \n", bitarray_test_bit(bitmap,k),k);
}

void setear_bloque_ocupado_en_posicion(t_bitarray* bitmap, off_t pos){
	bitarray_set_bit(bitmap, pos);
}

void setear_bloque_libre_en_posicion(t_bitarray* bitmap, off_t pos){
	bitarray_clean_bit(bitmap, pos);
}

bool testear_bloque_libre_en_posicion(t_bitarray* bitmap, int pos){
	return bitarray_test_bit(bitmap, (off_t)(pos));
}

// Obtiene y setea el proximo bloque libre
int getAndSetFreeBlock(t_bitarray* bitmap, unsigned int blocks){
	int j;
	for(j =0; testear_bloque_libre_en_posicion(bitmap, j); j++); // Hasta un bloque lbre
	setear_bloque_ocupado_en_posicion(bitmap, j);
	return j;
}

// Retorna la cantidad de bloques libres
int getFreeBlocks(int metadataBlocks, t_bitarray* bitmap){

    int bloques_libres = 0;
    int bloque_libre;
    int bit = 0;
    int tamMaximo = metadataBlocks;
    while(bit < tamMaximo)
    {
        bloque_libre = bitarray_test_bit(bitmap,bit);
        if(bloque_libre == 0)bloques_libres ++;
        bit++;
    }

    return bloques_libres;
}

/**
 * Setup
 * */
void createRootFiles() {
	char* dir_tallGrass = string_new();
	string_append(&dir_tallGrass, game_card_config->punto_montaje_tallgrass);

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
	struct_paths[TALL_GRASS] = dir_tallGrass;
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

	free(pokemonBasePath);
	free(pokemonBaseBin);
}

void createBlocks(){
	game_card_logger_info("Creando bloques en el path /Bloques");
	FILE* newBloque;
	for(int i=0; i <= lfsMetaData.blocks-1; i++){
        char* pathBloque = obtenerPathDelNumeroDeBloque(i);
        newBloque = fopen(pathBloque,"w+b");
        fclose(newBloque);
		free(pathBloque);
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
	config_set_value(config_metadata, "BLOCKS", "4096"); // asi no tengo 5492 bloques :P
	config_set_value(config_metadata, "MAGIC_NUMBER", "TALL_GRASS");
	config_save(config_metadata);
	config_destroy(config_metadata);
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

void readBitmap(char* bitmapBin) {
	game_card_logger_info("Leyendo Bitmap.bin");
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
