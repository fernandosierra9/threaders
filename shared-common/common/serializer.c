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
