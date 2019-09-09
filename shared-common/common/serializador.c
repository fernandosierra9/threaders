#include "serializador.h"

package_t* create_package(size_t size) {
	package_t* new_package = malloc(sizeof(package_t));

	new_package->load = malloc(size);
	new_package->size = size;
	new_package->remaining_load = size;

	return new_package;
}

void add_content(package_t* package, void* content, size_t content_size) {
	memcpy(package->load, content, content_size);

	package->load += content_size;
	package->remaining_load -= content_size;
}

void add_content_variable(package_t* package, void* content, size_t content_size) {
	add_content(package, &content_size, sizeof(size_t));

	add_content(package, content, content_size);
}

package_status check_package(package_t* package) {
	if (package->remaining_load > 0) {
		return LOAD_MISSING;
	}

	else if (package->remaining_load < 0) {
		return LOAD_EXTRA;
	}

	else {
		return LOAD_SUCCESS;
	}
}

void* build_package(package_t* package) {
	if(check_package(package) == LOAD_SUCCESS) {
		package->load -= package->size;

		void* serialized_package = malloc(package->size);

		memcpy(serialized_package, package->load, package->size);

		return serialized_package;
	}

	else {
		return NULL;
	}
}

package_status	send_serialized_package(int fd, void* serialized_package, size_t package_size) {
	int bytes_sent = send(fd, serialized_package, package_size, 0);

	if(bytes_sent < 0) {
		return SEND_ERROR;
	}

	else if(bytes_sent < package_size) {
		return send_serialized_package(fd, serialized_package + bytes_sent, package_size - bytes_sent);
	}

	else {
		return SEND_SUCCESS;
	}
}

package_t* receive_package(int socket_sender) {
	package_t *package = malloc(sizeof(package_t));

	int ret = recv(socket_sender, &(package->size), sizeof(package->size), MSG_WAITALL);
	if (ret == -1) {
		free(package);
		return NULL;
	}

	package->load = malloc(package->size);

	ret = recv(socket_sender, package->load, package->size, MSG_WAITALL);
	if (ret == -1) {
		free(package->load);
		free(package);
		return NULL;
	}

	return package;
}

void destroy_package(package_t* package) {
	free(package->load);
	free(package);
}

char* status_message(package_t* package, package_status status) {
	switch (status) {
		case LOAD_SUCCESS: return string_from_format("Se creo un paquete de tamaño %d\n", package->size); break;

		case SEND_SUCCESS: return string_duplicate("Se envio el paquete exitosamente\n"); break;

		case LOAD_MISSING: return string_from_format("Faltan completar %d de %d bytes para poder enviar el paquete\n", package->remaining_load, package->size); break;

		case LOAD_EXTRA: return string_from_format("Hay %d bytes de mas\n", abs(package->remaining_load)); break;

		case SEND_ERROR: return string_duplicate("No se pudo enviar el paquete\n"); break;

		default: return string_duplicate("Error inesperado\n"); break;
	}
}
