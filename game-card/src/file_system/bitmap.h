#ifndef BITMAP_H_
#define BITMAP_H_

#include <string.h>
#include <stdio.h>
#include <commons/bitarray.h>
#include <commons/txt.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <commons/bitarray.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "../logger/game_card_logger.h"

void mostrar_bitarray(t_bitarray* bitmap);
void setear_bloque_ocupado_en_posicion(t_bitarray* bitmap, off_t pos);
void setear_bloque_libre_en_posicion(t_bitarray* bitmap, off_t pos);
bool testear_bloque_libre_en_posicion(t_bitarray* bitmap, int pos);
int getAndSetFreeBlock(t_bitarray* bitmap, unsigned int blocks);
int getFreeBlocks(int metadataBlocks, t_bitarray* bitmap);

#endif /* BITMAP_H_ */
