#include "conector.h"

#include <errno.h>
#include <stdbool.h>

int connect_to_server(char *host, int port)
{
	struct sockaddr_in conexion_server;
	conexion_server.sin_family = AF_INET;
	conexion_server.sin_addr.s_addr = inet_addr(host);
	conexion_server.sin_port = htons(port);
	memset(&(conexion_server.sin_zero), 0, sizeof(conexion_server.sin_zero));

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(fd,
			(struct sockaddr *) &conexion_server,
			sizeof(conexion_server)) < 0) {
		return -1;
	}

	return fd;
}

int init_listener(int listen_port, int max_conn)
{
	int listener_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listener_fd == -1) {
		return NO_FD_ERROR;
	}

	int activate = 1;
	setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &activate, sizeof(activate));

	struct sockaddr_in listener_info;

	listener_info.sin_family = AF_INET;
	listener_info.sin_port = htons(listen_port);
	listener_info.sin_addr.s_addr = INADDR_ANY;
	memset(&(listener_info.sin_zero), 0, 8);

	if (bind(listener_fd, (struct sockaddr*) &listener_info, sizeof(struct sockaddr)) == -1) {
		return BIND_ERROR;
	}

	if (listen(listener_fd, max_conn) == -1) {
		return LISTEN_ERROR;
	}

	return listener_fd;
}

#define VERIFICATION_NUMBER 15418

int send_handshake(int fd, enum process_type type)
{
	int verification = VERIFICATION_NUMBER;
	if (send(fd, &verification, sizeof(verification), 0) != sizeof(verification)) {
		return 0;
	}

	if (send(fd, &type, sizeof(type), 0) == sizeof(type)) {
		return 1;
	} else {
		return 0;
	}
}

int receive_handshake(int fd)
{
	int verification;
	if (recv(fd, &verification, sizeof(verification), MSG_WAITALL) != sizeof(verification)) {
		return -1;
	} else if (verification != VERIFICATION_NUMBER) {
		return -1;
	}

	enum process_type type;
	if (recv(fd, &type, sizeof(type), MSG_WAITALL) == sizeof(type)) {
		return type;
	} else {
		return -1;
	}
}

int send_confirmation(int fd, bool confirm) {
	if (send(fd, &confirm, sizeof(confirm), 0) == sizeof(confirm)) {
		return 1;
	} else {
		return 0;
	}
}

int receive_confirmation(int fd, bool *confirm) {
	if (recv(fd, confirm, sizeof(*confirm), MSG_WAITALL) == sizeof(*confirm)) {
		return 1;
	} else {
		return 0;
	}
}
