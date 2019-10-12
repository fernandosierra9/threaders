#ifndef COMMON_PROTOCOLS_H_
#define COMMON_PROTOCOLS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef enum
{
	HANDSHAKE, MALLOC, FREE_MALLOC, COPY, GET, MAP, UNMAP, SYNC, MALLOC_OK, FREE_OK, GET_OK,
	MAP_OK, UNMAP_OK, SYNC_OK, SEG_FAULT
} t_protocol;

typedef struct
{
	uint32_t ptr;
} t_malloc_ok;

typedef struct
{
	void * res;
	int tamres;
} t_get_ok;

// TODO: Implement the rest of the functions

typedef struct
{
	t_protocol operation_code;
	t_buffer* buffer;
} t_package;

typedef struct
{
	uint32_t tam;
	int id_libmuse;
} t_malloc;

typedef struct
{
	uint32_t src;
	int id_libmuse;
	int size;
} t_get;


#endif /* COMMON_PROTOCOLS_H_ */
