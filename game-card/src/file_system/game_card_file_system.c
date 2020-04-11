#include "game_card_file_system.h"

t_list* files;

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
	if((f_metadata = fopen(bin_metadata, "r")) == NULL)
	{
		game_card_logger_warn("TALL_GRASS no encontrado, se creara uno nuevo");
		f_metadata = fopen(bin_metadata, "wb+");
		config_metadata = config_create(bin_metadata);
		config_set_value(config_metadata, "BLOCK_SIZE", "64");
		config_set_value(config_metadata, "BLOCKS", "5192");
		config_set_value(config_metadata, "MAGIC_NUMBER", "TALL_GRASS");
		config_save(config_metadata);
	}
	else
	{
		config_metadata = config_create(bin_metadata);
	}
	fclose(f_metadata);
	game_card_logger_info("Creado el archivo Metadata.bin");

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

void gcfs_free_bitmap()
{
	free(bitmap->bitarray);
	bitarray_destroy(bitmap);
}
