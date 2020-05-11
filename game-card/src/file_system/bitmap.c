#include "bitmap.h"

void mostrar_bitarray(t_bitarray* bitmap){
	for(int k =0;k<(bitarray_get_max_bit(bitmap));k++)
	printf("test bit posicion, es  %d en posicion %d \n", bitarray_test_bit(bitmap,k),k);
}
