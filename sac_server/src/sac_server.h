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
#include <commons/collections/list.h>
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

#define THELARGESTFILE (uint32_t) (BLOCK_INDIRECT*PTR_BLOCK_SIZE*BLOCK_SIZE)

// Utils de Bloques
#define NODE_TABLE_SIZE 1024
#define DISC_PATH fuse_disc_path
#define DISC_SIZE_B(p) path_size_in_bytes(p)
#define ACTUAL_DISC_SIZE_B fuse_disc_size
#define BITMAP_BLOCK_SIZE header_data.size_bitmap
#define BITMAP_SIZE_B (int) (get_size() / CHAR_BIT)
#define BITMAP_SIZE_BITS get_size()

// Se utiliza esta variable para saber si se encuentra en modo "borrar". Esto afecta, principalmente, al delete_nodes_upto
#define DELETE_MODE _del_mode
#define ENABLE_DELETE_MODE _del_mode=1
#define DISABLE_DELETE_MODE _del_mode=0
int _del_mode;

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
char *fuse_disc_path;
// Tamaño del disco.
int fuse_disc_size;
// Se guardara aqui la cantidad de bloques libres en el bitmap
int bitmap_free_blocks;

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
int delete_nodes_upto(struct sac_file_t *file_data, int pointer_upto, int data_upto);
int set_position (int *pointer_block, int *data_block, size_t size, off_t offset);
int get_node(void); // ---> REVISAR IMPORTANTEEEEEEEEE
int obtain_free_blocks(void);
int add_node(struct sac_file_t *file_data, int node_number);


// Funciones de escritura
int sac_server_create_directory(const char *path);
int sac_server_remove_directory(const char* path);
int sac_server_unlink_node(const char* path);
int sac_server_make_node(const char* path);
int sac_server_write(const char *path, const char *buf, size_t size, off_t offset);
int sac_server_flush();

// Funciones de lectura
int sac_server_readdir(const char *path, t_list* nodes);
int sac_server_read(const char *path, char **buf, size_t* size, off_t* offset);
int sac_server_open(const char *path, struct fuse_file_info *fi);
struct sac_file_t* sac_server_getattr(const char *path, int* res);

#endif /* SAC_SERVER_H_ */
