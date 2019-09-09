#ifndef LIBS_SERIALIZADOR_H_
#define LIBS_SERIALIZADOR_H_

#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

typedef struct {
	void*	load;
	size_t	size;
	size_t	remaining_load;
} package_t;

typedef enum {
	LOAD_SUCCESS,
	LOAD_MISSING,
	LOAD_EXTRA,
	SEND_SUCCESS,
	SEND_ERROR }
package_status;

package_t*			create_package(size_t size);

void				add_content(package_t* package, void* content, size_t content_size);

void				add_content_variable(package_t* package, void* content, size_t content_size);

void				add_content_list(package_t* package, t_list* list, size_t (*size_calculator)(void*));

package_status		check_package(package_t* package);

void* 				build_package(package_t* package);

package_status		send_serialized_package(int fd, void* serialized_package, size_t package_size);

void				destroy_package(package_t* package);

char*				status_message(package_t* package, package_status status);

// Deprecated

package_t*			receive_package(int socket_sender);

#endif
