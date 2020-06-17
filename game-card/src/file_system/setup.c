#include "setup.h"

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
