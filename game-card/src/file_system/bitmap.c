#include "bitmap.h"

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
	if(j>blocks) {
		printf("cantidad insuficiente de espacio o bloques ");
	}
	setear_bloque_ocupado_en_posicion(bitmap, j);
	return j;
}