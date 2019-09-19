#include "sockets.h"

struct addrinfo* set_server_info(char* ip, int port)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));

	// No importa si uso IPv4 o IPv6
	hints.ai_family = AF_UNSPEC;
	// Asigna el address del localhost: 127.0.0.1
	hints.ai_flags = AI_PASSIVE;
	// Indica que usaremos el protocolo TCP
	hints.ai_socktype = SOCK_STREAM;

	// Si IP es NULL, usa el localhost
	getaddrinfo(ip, string_itoa(port), &hints, &server_info);
	return server_info;
}

int socket_create_client(char* ip, int port)
{
	if (ip == NULL)
		return -1;
	struct addrinfo* server_info = set_server_info(ip, port);

	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);


	if (server_socket ==-1){
		perror("-1 al crear el socket");
		return -1;
	}

	int activado = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado))==-1){
		perror("-1 al setear las opciones del socket");
		return -1;
	}
	return server_socket;
}
int socket_create_listener(char* ip, int port){
		if (ip == NULL)
			return -1;
		struct addrinfo* server_info = set_server_info(ip, port);

		int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

		int activado = 1;
		setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

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




int socket_connect_to_server(char* ip, int port, int server_socket)
{
	struct addrinfo* server_info = set_server_info(ip, port);
	int retorno = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return (retorno < 0 || server_socket == -1) ? -1 : server_socket;
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
	char ip_nodo[20];
	strcpy(ip_nodo, inet_ntoa(addr.sin_addr));
	return strdup(ip_nodo);
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
