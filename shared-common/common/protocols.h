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
	HANDSHAKE,
	MALLOC,
	FREE_MALLOC,
	COPY,
	GET,
	OPEN
} t_protocol;

typedef struct
{
	t_protocol operation_code;
	t_buffer* buffer;
} t_package;

typedef struct
{
	uint32_t memoria;
	int id_libmuse;
} t_malloc;

typedef struct
{
	uint32_t src;
	int id_libmuse;
	int size;
} t_get;


#endif /* COMMON_PROTOCOLS_H_ */
