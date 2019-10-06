#ifndef SAC_CLI_H_
#define SAC_CLI_H_

    #include <stdint.h>
    #include <stddef.h>
	#include "../shared-common/common/sockets.h"
	#include "../shared-common/common/utils.h"

    int muse_init(int id);
    void muse_close();
    uint32_t muse_alloc(uint32_t tam);
    void muse_free(uint32_t dir);
    int muse_get(void* dst, uint32_t src, size_t n);
    int muse_cpy(uint32_t dst, void* src, int n);
    uint32_t muse_map(char *path, size_t length, int flags);


#endif
