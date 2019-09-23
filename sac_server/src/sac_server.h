#ifndef SAC_SERVER_H_
#define SAC_SERVER_H_

/* Opciones de FUSE. Esta redefinicion le indica cuales son las opciones que se utilizaran. */
#define FUSE_USE_VERSION 27
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include "config/sac_server_config.h"
#include "logger/sac_server_logger.h"
#include "operations/read/sac_read.h"
#include "operations/write/sac_write.h"
#include "operations/operate/sac_operate.h"

#define SAC_FILE_BY_TABLE 1024
#define SAC_FILE_BY_BLOCK 1
#define SAC_FILE_NAME_LENGTH 71
#define SAC_HEADER_BLOCKS 1
#define BLOCK_INDIRECT 1000
#define BLOCK_SIZE 4096
#define PTR_BLOCK_SIZE 1024

typedef uint32_t ptr_sac_block;

typedef ptr_sac_block pointer_data_block [PTR_BLOCK_SIZE];

/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }

//Un bloque
typedef struct sac_header_t {
	unsigned char id[3];
	uint32_t version;
	uint32_t block_bitmap;
	uint32_t size_bitmap; // en bloques
	unsigned char padding[4081];
} SHeader;

struct sac_header_t header_data;

typedef struct sac_file_t {
	uint8_t state;
	unsigned char file_name[SAC_FILE_NAME_LENGTH];
	uint32_t parent_dir_block;
	uint32_t file_size;
	uint64_t creation_date;
	uint64_t modified_date;
	ptr_sac_block block_indirect[BLOCK_INDIRECT];
} SFile;

#endif /* SAC_SERVER_H_ */
