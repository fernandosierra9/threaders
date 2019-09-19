#ifndef LIBS_SERIALIZADOR_H_
#define LIBS_SERIALIZADOR_H_

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "protocols.h"

#define RECV_DATA_ERROR		-1
#define RECV_SIZE_ERROR		-2
#define RECV_MALLOC_ERROR	-3
#define RECV_SUCCESS		1

package_t* create_package(size_t size);

void add_content(package_t* package, void* content, size_t content_size);

void add_content_variable(package_t* package, void* content,
		size_t content_size);

void add_content_list(package_t* package, t_list* list,
		size_t (*size_calculator)(void*));

package_status check_package(package_t* package);

void* build_package(package_t* package);

//package_status send_serialized_package(int fd, void* serialized_package,
//		size_t package_size);

void destroy_package(package_t* package);

char* status_message(package_t* package, package_status status);

// Deprecated

//package_t* receive_package(int socket_sender);
//
//int recv_package(int fd, void* receiver, size_t size);

//int recv_package_variable(int fd, void** receiver);

void* serializer_serialize_package(t_package* package, int bytes);

#endif
