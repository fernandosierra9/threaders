#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "deserializador.h"

#define RECV_DATA_ERROR		-1
#define RECV_SIZE_ERROR		-2
#define RECV_MALLOC_ERROR	-3
#define RECV_SUCCESS		1

int recv_package(int fd, void* receiver, size_t size) {
	bool success = recv(fd, receiver, size, MSG_WAITALL) == size;
	return success ? RECV_SUCCESS : RECV_DATA_ERROR;
}

int recv_package_variable(int fd, void** receiver) {
	size_t package_size;
	if (!recv_package(fd, &package_size, sizeof(package_size))) {
		return RECV_SIZE_ERROR;
	}

	*receiver = malloc(package_size);
	if (receiver == NULL) {
		return RECV_MALLOC_ERROR;
	}

	return recv_package(fd, *receiver, package_size);
}
