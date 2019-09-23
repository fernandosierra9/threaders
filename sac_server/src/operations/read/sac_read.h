#ifndef SAC_READ_H_
#define SAC_READ_H_

#include "../../sac_server.h"

#define DEFAULT_FILE_CONTENT "Hello World!\n"
#define DEFAULT_FILE_NAME "hello"
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

int sac_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int sac_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int sac_open(const char *path, struct fuse_file_info *fi);
int sac_getattr(const char *path, struct stat *stbuf);

#endif /* SAC_READ_H_ */
