#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libmuse.h"
#include "common/sockets.h"

char *ip_muse = "127.0.0.1";
int puerto_muse = 5003;
int muse_fd;

int muse_init(int id)
{
    printf("HELLO MUSE");
    muse_fd = socket_connect_to_server(ip_muse, puerto_muse);

	if(muse_fd < 0)
	{
		close(muse_fd);
		return -1;
	}
	puts("Conexion con Muse establecida");

	t_malloc *sendMalloc = malloc(sizeof(t_malloc));
	sendMalloc->memoria = 2000;
	t_protocol malloc_protocol = MALLOC;
	utils_serialize_and_send(muse_fd, malloc_protocol, sendMalloc);

	return 0;
}

void muse_close(){ /* Does nothing :) */ }

uint32_t muse_alloc(uint32_t tam){
    return (uint32_t) malloc(tam);
}

void muse_free(uint32_t dir) {
    free((void*) dir);
}

int muse_get(void* dst, uint32_t src, size_t n){
    memcpy(dst, (void*) src, n);
    return 0;
}

int muse_cpy(uint32_t dst, void* src, int n){
    memcpy((void*) dst, src, n);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
uint32_t muse_map(char *path, size_t length, int flags){
    return 0;
}

int muse_sync(uint32_t addr, size_t len){
    return 0;
}

int muse_unmap(uint32_t dir){
    return 0;
}
