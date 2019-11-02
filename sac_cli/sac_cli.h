#ifndef SAC_CLI_H_
#define SAC_CLI_H_

/* Opciones de FUSE. Esta redefinicion le indica cuales son las opciones que se utilizaran. */
#define FUSE_USE_VERSION 27
#define _FILE_OFFSET_BITS 64

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stddef.h>
#include "../shared-common/common/sockets.h"
#include "../shared-common/common/utils.h"


char *ip_sac_server = "127.0.0.1";
int puerto_sac = 8003;
int sac_cli_fd;

// Ruta del disco.
char fuse_disc_path[1000];

#define DEFAULT_FILE_CONTENT "Hello World!\n"
#define DEFAULT_FILE_NAME "hello"
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }


int sac_cli_init(int argc, char *argv[]);
int sac_cli_create_directory(const char *path, mode_t mode);
int sac_cli_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int sac_cli_read(char *path);
// int sac_cli_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int sac_cli_open(const char *path, struct fuse_file_info *fi);
int sac_cli_getattr(const char *path, struct stat *stbuf);
int sac_cli_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int sac_cli_rm_directory (const char* path);

#endif
