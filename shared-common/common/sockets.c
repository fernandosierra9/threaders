#include "sockets.h"

int socket_create_listener(char* ip, int port)
{
	if (ip == NULL)
		return -1;

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, string_itoa(port), &hints, &server_info);

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int activated = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &activated, sizeof(activated));

	if (server_socket == -1 || bind(server_socket, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		freeaddrinfo(server_info);
		return -1;
	}

	freeaddrinfo(server_info);

	if (listen(server_socket, BACKLOG) == -1)
		return -1;
	return server_socket;
}

int socket_connect_to_server(char* ip, int port)
{
	if (ip == NULL)
		return -1;

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, string_itoa(port), &hints, &server_info);

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int result = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return (result < 0 || server_socket == -1) ? -1 : server_socket;
}

int socket_accept_conection(int server_socket)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int client_socket = accept(server_socket, (struct sockaddr *) &addr, &addrlen);
	if (client_socket < 0)
	{
		perror("Error al aceptar cliente");
		return -1;
	}
	return client_socket;
}

char* socket_get_ip(int fd)
{
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	int res = getpeername(fd, (struct sockaddr *) &addr, &addr_size);
	if (res == -1)
		return NULL;
	char ip_node[20];
	strcpy(ip_node, inet_ntoa(addr.sin_addr));
	return strdup(ip_node);
}

void socket_close_conection(int socket_client)
{
	close(socket_client);
}

int escuchar(int socketListener, fd_set *fd,
				void *(funcionSocketNuevo)(int, void*),
				void *argumentosSocketNuevo,
				void *(funcionSocketExistente)(int, void*),
				void *argumentosSocketExistente)
{

	fd_set copia = *fd;
	int socketsComunicandose = 0;
	if ((socketsComunicandose = select(FD_SETSIZE, &copia, NULL, NULL, NULL))
					== -1)
	{

		perror("Fallo en el select");
		return -1;
	}

	if (FD_ISSET(socketListener, &copia))
	{

		int socketNuevo = 0;
		if ((socketNuevo = accept(socketListener, NULL, 0)) < 0)
		{

			perror("Error al aceptar conexion entrante");
			return -1;
		}
		else
		{

			FD_SET(socketNuevo, fd);

			if (funcionSocketNuevo != NULL)
			{
				funcionSocketNuevo(socketNuevo, argumentosSocketNuevo);
			}
		}
	}
	else
	{

		int i;
		for (i = 0; i < FD_SETSIZE; i++)
		{

			if (FD_ISSET(i, &copia) && i != socketListener)
			{
				funcionSocketExistente(i, argumentosSocketExistente);
			}
		}
	}
	return 0;
}
