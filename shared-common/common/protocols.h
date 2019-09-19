#ifndef COMMON_PROTOCOLS_H_
#define COMMON_PROTOCOLS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum
{
	MESSAGE,
	PACKAGE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code operation_code;
	t_buffer* buffer;
} t_package;

typedef struct
{
	void* load;
	size_t size;
	size_t remaining_load;
} package_t;

typedef enum
{
	LOAD_SUCCESS, LOAD_MISSING, LOAD_EXTRA, SEND_SUCCESS, SEND_ERROR
} package_status;

typedef enum
{
	HANDSHAKE,
	MALLOC,
	FREE_MALLOC,
	COPY,
	GET
} t_protocolo;

typedef struct
{
	uint32_t memoria;
} t_malloc;

typedef struct
{
	t_protocolo operation_code;
	t_buffer* buffer;
} t_paquete;

#endif /* COMMON_PROTOCOLS_H_ */
