#include "libmuse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int muse_init(int id){
    printf("HELLO MUSE");
    int muse_fd;
    crearSocket(&muse_fd);
    int puerto_muse = 5003;
    char *ip_muse="127.0.0.1";


	if(conectar(&muse_fd,puerto_muse,ip_muse)!=0){
		return -1;
	}
	else{
		puts("Conexion con Muse establecida");
		return 0;
	}
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
