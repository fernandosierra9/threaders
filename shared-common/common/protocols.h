#ifndef COMMON_PROTOCOLS_H_
#define COMMON_PROTOCOLS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef enum {
	HANDSHAKE,
	MALLOC,
	MEMFREE,
	COPY,
	GET,
	MAP,
	UNMAP,
	SYNC,
	MALLOC_OK,
	FREE_RESPONSE,
	GET_OK,
	COPY_RESPONSE,
	MAP_OK,
	UNMAP_OK,
	SYNC_OK,
	SEG_FAULT
} t_protocol;

typedef struct {
	t_protocol operation_code;
	t_buffer* buffer;
} t_package;

typedef struct {
	uint32_t tam;
	int id_libmuse;
} t_malloc;

typedef struct {
	uint32_t src;
	int id_libmuse;
	int size;
} t_get;

typedef struct {
	uint32_t dst;
	int self_id;
	int size;
} t_copy;

typedef struct {
	uint32_t dir;
	int self_id;
} t_free;

typedef struct {

} t_map;

typedef struct {

} t_unmap;

typedef struct {

} t_sync;

typedef struct {
	uint32_t ptr;
} t_malloc_ok;

typedef struct {
	int res;
} t_free_response;

typedef struct {
	void * res;
	int tamres;
} t_get_ok;

typedef struct {
	int res;
} t_copy_response;

typedef struct {

} t_map_ok;

typedef struct {

} t_unmap_ok;

typedef struct {

} t_sync_ok;

#endif /* COMMON_PROTOCOLS_H_ */
