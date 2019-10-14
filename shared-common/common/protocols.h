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
	SEG_FAULT,
	GET_ATTR,
	READ_DIR,
	READ,
	OPEN,
	MK_DIR,
	CREATE_DIR,
	WRITE,
	RM_DIR
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
  // TODO: Implementation
} t_map;

typedef struct {
  // TODO: Implementation
} t_unmap;

typedef struct {
  // TODO: Implementation
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
  // TODO: Implementation
} t_map_ok;

typedef struct {
  // TODO: Implementation
} t_unmap_ok;

typedef struct {
  // TODO: Implementation
} t_sync_ok;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_read_dir;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_read;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_open;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_get_attr;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_create_directory;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_write;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_rm_directory;

typedef struct {
	char* pathname;
	int id_sac_cli;
} t_mk_directory;

#endif /* COMMON_PROTOCOLS_H_ */
