#include "../../hilolay/hilolay.h"
#include "../../libmuse/libmuse.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

uint32_t leer_archivo(uint32_t arch, uint32_t leido)
{
	uint32_t len;
	char * palabra = malloc(100);

	//muse_get(&len, arch + leido, sizeof(uint32_t));
	//leido += sizeof(uint32_t);
	muse_get(palabra, arch + leido, 10);
	leido += len;

	printf("string %s",palabra);
	free(palabra);
	return leido;
}

void *revolucionar()
{
	uint32_t arch = muse_map("/home/utnso/git/tp-2019-2c-threaders/muse/Debug/fernando.txt", 4096, MAP_PRIVATE);
	uint32_t offset = 0;
	uint32_t size;

	//muse_get(&size, arch, sizeof(uint32_t));
	offset = sizeof(uint32_t);
	//while(offset < size)
	offset = leer_archivo(arch, 30);

	arch += 5000;

	muse_get(NULL, arch, 1);

	muse_sync(arch, 4096);

	muse_unmap(arch);
	return 0;
}


int main(void)
{
	//struct hilolay_t revolucion;

	//hilolay_init();
	muse_init(2, "127.0.0.1", 5003);
	uint32_t arch = muse_map("/home/utnso/git/tp-2019-2c-threaders/muse/Debug/fernando.txt", 4096, MAP_PRIVATE);
	char * palabra = malloc(100);
	muse_get(palabra, arch +5000 , 10);
	printf("algo");
	printf("%s",palabra);
	muse_sync(arch, 4096);
	muse_unmap(arch);

}
