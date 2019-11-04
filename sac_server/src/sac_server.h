#ifndef SAC_SERVER_H_
#define SAC_SERVER_H_

// ELIMINAR ESTO
#define FUSE_USE_VERSION 27
#define _FILE_OFFSET_BITS 64
#include <fuse.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include "config/sac_server_config.h"
#include "logger/sac_server_logger.h"
#include "../../shared-common/common/utils.h"
#include "../../shared-common/common/sockets.h"

#define DEFAULT_FILE_CONTENT "Hello World!\n"
#define DEFAULT_FILE_NAME "hello"
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

#define SAC_FILE_BY_TABLE 1024
#define SAC_FILE_BY_BLOCK 1
#define SAC_FILE_NAME_LENGTH 71
#define SAC_HEADER_BLOCKS 1
#define BLOCK_INDIRECT 1000
#define BLOCK_SIZE 4096
#define PTR_BLOCK_SIZE 1024

// Utils de Bloques
#define NODE_TABLE_SIZE 1024
#define DISC_PATH fuse_disc_path
#define DISC_SIZE_B(p) path_size_in_bytes(p)
#define ACTUAL_DISC_SIZE_B fuse_disc_size
#define BITMAP_BLOCK_SIZE header_data.size_bitmap

// Definiciones de tipo de bloque borrado(0), archivo(1), directorio(2)
#define DELETED_T ((int) 0)
#define FILE_T ((int) 1)
#define DIRECTORY_T ((int) 2)

typedef uint32_t pointerSACBlock;

typedef pointerSACBlock pointer_data_block [PTR_BLOCK_SIZE];

// Se guarda el tamaño del bitarray para la implementacion de 64 bits.
#define ARRAY64SIZE _bitarray_64
size_t _bitarray_64;
#define ARRAY64LEAK _bitarray_64_leak
size_t _bitarray_64_leak;

// Ruta del disco.
char fuse_disc_path[1000];
// Tamaño del disco.
int fuse_disc_size;

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
	pointerSACBlock block_indirect[BLOCK_INDIRECT];
} SFile;


// Utilizadas para mmap
struct sac_header_t *header_start;
struct sac_file_t *node_table_start, *data_block_start, *bitmap_start;

int discDescriptor;

// Funciones auxiliares
int path_size(const char* path);
pointerSACBlock determinar_nodo(const char* path);
int split_path(const char* path, char** super_path, char** name);
int get_size(void);


// Funciones de escritura
int sac_server_create_directory(const char *path, mode_t mode);

// Funciones de lectura
int sac_server_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int sac_server_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int sac_server_open(const char *path, struct fuse_file_info *fi);
int sac_server_getattr(const char *path, struct stat *stbuf);


#endif /* SAC_SERVER_H_ */
