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
	FREE_MALLOC,
	COPY,
	GET,
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
	uint32_t memoria;
	int id_libmuse;
} t_malloc;

typedef struct {
	uint32_t src;
	int id_libmuse;
	int size;
} t_get;

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
