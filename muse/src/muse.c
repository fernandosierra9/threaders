#include "muse.h"

void muse_server_init();

int main(void) {
	int response = muse_logger_create();
	if (response < 0)
		return response;

	response = muse_config_load();
	if (response < 0) {
		muse_logger_destroy();
		return response;
	}
	muse_server_init();
	muse_config_free();
	muse_logger_destroy();
	return EXIT_SUCCESS;
}

void muse_server_init() {
	muse_socket = socket_create_listener("127.0.0.1", muse_get_listen_port());
	if (muse_socket < 0) {
		muse_logger_error("Error al crear server");
		return;
	}

	muse_logger_info("Esperando conexion del libmuse");

	int libmuse_fd = socket_accept_conection(muse_socket);

	if (libmuse_fd == -1) {
		muse_logger_error("Error al establecer conexion con el libmuse");
		return;
	}

	muse_logger_info("Conexion establecida con libmuse");
	int received_bytes;
	int protocol;
	while (1) {
		received_bytes = recv(libmuse_fd, &protocol, sizeof(int), 0);

		if (received_bytes <= 0) {
			muse_logger_error("Error al recibir la operacion del libmuse");
			muse_logger_error("Se desconecto libmuse");
			exit(1);
		}
		switch (protocol) {
		case HANDSHAKE: {
			muse_logger_info("Hi");
			break;
		}
		case MALLOC: {
			muse_logger_info("Recibi malloc de libmuse");
			t_malloc *malloc_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);
			muse_logger_info("MALLOC_TAM: %d", malloc_receive->tam);
			muse_logger_info("ID_LIBMUSE: %d", malloc_receive->id_libmuse);
			// Response logic
			t_malloc_ok* res = malloc(sizeof(t_malloc_ok));
			res->ptr = 100;
			t_protocol malloc_protocol = MALLOC_OK;
			utils_serialize_and_send(libmuse_fd, malloc_protocol, res);
			break;
		}
		case FREE_MALLOC: {
			muse_logger_info("Recibi malloc de libmuse");
			t_malloc *free_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);
			break;
		}
		case GET: {
			muse_logger_info("Recibi get de libmuse");
			t_get *get_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);
			// Response logic
			t_get_ok* response = malloc(sizeof(t_get_ok));
			response->res = 120;
			response->tamres = 4;
			utils_serialize_and_send(libmuse_fd, protocol, response);
			break;
		}
		case COPY: {
			// TODO: Implementation
		}
		case SYNC: {
			// TODO: Implementation
		}
		case MAP: {
			// TODO: Implementation
		}
		case UNMAP: {
			// TODO: Implementation
		}
		}
	}
}
