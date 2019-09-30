#include "sac_operate.h"

// Size del disco
int path_size(const char* path){
	FILE *fd;
	int size;
	fd=fopen(path, "r");
	fseek(fd, 0L, SEEK_END);
	size = ftell(fd);
	fclose(fd);
	return size;
}