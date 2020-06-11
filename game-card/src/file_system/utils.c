
char* crearPathBloque(int bloque, char* montajeBloques) {
    char* nroBloque = string_duplicate(montajeBloques);
    char* str_nroBloque = string_itoa(bloque);
    string_append(&nroBloque, str_nroBloque);
    string_append(&nroBloque, ".bin");

    return nroBloque;
}