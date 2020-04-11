#include "serializer.h"

void* serializer_serialize_package(t_package* package, int bytes)
{
	void * magic = malloc(bytes);
	int offset = 0;

	memcpy(magic + offset, &(package->operation_code), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, &(package->buffer->size), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, package->buffer->stream, package->buffer->size);
	offset += package->buffer->size;

	return magic;
}

t_package* serializer_deserialize_object(void* object, int bytes)
{
	t_package* magic_object = (t_package*)object;
	t_package* magic = malloc(bytes);
	int offset = 0;

	memcpy(magic + offset, &(magic_object->operation_code), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, &(magic_object->buffer->size), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, magic_object->buffer->stream, magic_object->buffer->size);
	offset += magic_object->buffer->size;

	return magic;
}
