#include "bloques_handler.h"

int cuantosBloquesOcupa(char* value) {

    int tamanio = string_length(value);

    return calcualarBloques(tamanio);
}

int calcualarBloques(int tamanio) {
    // Redondea hacia arriba
    return 1 + ((tamanio - 1) / lfsMetaData.blockSize);
}