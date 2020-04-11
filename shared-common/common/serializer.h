#ifndef LIBS_SERIALIZADOR_H_
#define LIBS_SERIALIZADOR_H_

#include <string.h>
#include <stdlib.h>
#include "protocols.h"

void* serializer_serialize_package(t_package* package, int bytes);
t_package* serializer_deserialize_object(void* object, int bytes);

#endif
