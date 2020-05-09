#include "game_card_file_system.h"

t_list* files;

static void mostrar_bitarray(){
	for(int k =0;k<(bitarray_get_max_bit(bitmap));k++)printf("test bit posicion, es  %d en posicion %d \n", bitarray_test_bit(bitmap,k),k);
}

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

void gcfs_create_structs()
{
	files = malloc(sizeof(t_list));
	files = list_create();

	char* dir_metadata = string_new();
	char* archivos = string_new();
	char* dir_bloques = string_new();
	char* bin_metadata = string_new();
	char* bin_bitmap = string_new();
	FILE* f_metadata;

	if(_mkpath(game_card_config->punto_montaje_tallgrass, 0755) == -1)
	{
		game_card_logger_error("_mkpath");
	}

	string_append(&dir_metadata, game_card_config->punto_montaje_tallgrass);
	string_append(&dir_metadata, "Metadata/");
	mkdir(dir_metadata, 0777);
	game_card_logger_info("Creada carpeta Metadata");

	string_append(&archivos, game_card_config->punto_montaje_tallgrass);
	string_append(&archivos, "Files/");
	mkdir(archivos, 0777);
	game_card_logger_info("Creada carpeta Archivos");

	string_append(&dir_bloques, game_card_config->punto_montaje_tallgrass);
	string_append(&dir_bloques, "Bloques/");
	mkdir(dir_bloques, 0777);
	game_card_logger_info("Creada carpeta Bloques");

	string_append(&bin_metadata, dir_metadata);
	string_append(&bin_metadata, "/Metadata.bin");
	
	readMetaData();
	createBlocks();

	string_append(&bin_bitmap, dir_metadata);
	string_append(&bin_bitmap, "/Bitmap.bin");
	if((bitmap_file = fopen(bin_bitmap, "rb+")) == NULL)
	{
		bitmap_file = fopen(bin_bitmap, "wb+");
		char* bitarray_limpio_temp = calloc(1, ceiling(config_get_int_value(config_metadata, "BLOCKS"), 8));
		fwrite((void*) bitarray_limpio_temp, ceiling(config_get_int_value(config_metadata, "BLOCKS"), 8), 1, bitmap_file);
		fflush(bitmap_file);
		free(bitarray_limpio_temp);
	}
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
	game_card_logger_info("Creado el archivo Bitmap.bin");

	struct_paths[METADATA] = dir_metadata;
	struct_paths[FILES] = archivos;
	struct_paths[BLOCKS] = dir_bloques;

	free(bin_metadata);
	free(bin_bitmap);
}


void createBlocks(){

	game_card_logger_info("Creando bloques en el path /Bloques");
	char* blocksPath = malloc(strlen(game_card_config->punto_montaje_tallgrass)+strlen("Bloques/"));
	strcpy(blocksPath, game_card_config->punto_montaje_tallgrass);
	strcat(blocksPath, "Bloques/");
	FILE * newBloque;
    
	for(int i=0; i <= lfsMetaData.blocks-1; i++){
        char* nroBloque = string_new();
        string_append(&nroBloque, blocksPath);
        string_append(&nroBloque, string_itoa(i));
        string_append(&nroBloque, ".bin");
        newBloque = fopen(nroBloque,"w+b");
        fclose(newBloque);
        free(nroBloque);
    }
}


void readMetaData() {
	game_card_logger_info("Leyendo el archivo Metadata.bin");
	char *metadata_path = (char *) malloc(1 + strlen(game_card_config->punto_montaje_tallgrass) + strlen("/Metadata/Metadata.bin"));;
	strcpy(metadata_path, game_card_config->punto_montaje_tallgrass);
	string_append(&metadata_path,"/Metadata/Metadata.bin");
	t_config* metadataFile = config_create(metadata_path);

	lfsMetaData.blocks = config_get_int_value(metadataFile,"BLOCKS");
    lfsMetaData.magicNumber = string_duplicate(config_get_string_value(metadataFile,"MAGIC_NUMBER"));
	lfsMetaData.blockSize = config_get_int_value(metadataFile,"BLOCK_SIZE");
	
	free(metadata_path);
	config_destroy(metadataFile);
}

void gcfs_free_bitmap()
{
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);
}
