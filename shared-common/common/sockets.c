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

void socket_select_init_clients(int client_socket[])
{
	//initialize all client_socket[] to 0 so not checked
	int i;
	for (i = 0; i < BACKLOG; i++)
	{
		client_socket[i] = 0;
	}
}

void socket_select_add_child_sockets_to_set(int sd, int max_sd, int client_socket[], fd_set readfds)
{
	//add child sockets to set
	int i;
	for (i = 0; i < BACKLOG; i++)
	{
		//socket descriptor
		sd = client_socket[i];
		//if valid socket descriptor then add to read list
		if (sd > 0)
			FD_SET(sd, &readfds);
		//highest file descriptor number, need it for the select function
		if (sd > max_sd)
			max_sd = sd;
	}
}

void socket_select_add_new_client(int new_socket, int client_socket[])
{
	//add new socket to array of sockets
	int i;
	for (i = 0; i < BACKLOG; i++)
	{
		//if position is empty
		if (client_socket[i] == 0)
		{
			client_socket[i] = new_socket;
			break;
		}
	}
}

void socket_select_catch_incoming_conections(int master_socket,int client_socket[], fd_set readfds)
{
	//If something happened on the master socket, then its an incoming connection
	if (FD_ISSET(master_socket, &readfds))
	{
		int new_socket = socket_accept_conection(master_socket);
		if (new_socket < 0)
		{
			exit(EXIT_FAILURE);
		}

		/**
		 * Here should go code/function to validate what to do like handshake, etc*/
		printf("IP cliente: %s", socket_get_ip(new_socket));

		socket_select_add_new_client(new_socket, client_socket);
	}
}

void socket_select_close_and_reuse(int i, int sd, int client_socket[])
{
	//Close the socket and mark as 0 in list for reuse
	socket_close_conection(sd);
	client_socket[i] = 0;
}

void socket_select_check_operations(int sd, int client_socket[], fd_set readfds)
{
	//else its some IO operation on some other socket
	int i;
	for (i = 0; i < BACKLOG; i++)
	{
		sd = client_socket[i];

		if (FD_ISSET(sd, &readfds))
		{
			//Check if it was for closing , and also read the incoming message

			/**
			 *Here should go another method when closing and receiving a message
			 * */
			if (1 == 0)
			{
				//Somebody disconnected , get his details and print
				printf("IP cliente desconectado: %s", socket_get_ip(sd));
				socket_select_close_and_reuse(i, sd, client_socket);
			}

			//Echo back the message that came in
			else
			{

			}
		}
	}
}

void socket_start_listening_select(char* ip, int port)
{
	int master_socket, client_socket[BACKLOG];
	fd_set readfds;

	if ((master_socket = socket_create_listener(ip, port)) < 0)
	{
		exit(EXIT_FAILURE);
	}

	socket_select_init_clients(client_socket);
	while (1)
	{
		//clear the socket set
		FD_ZERO(&readfds);
		//add master socket to set
		FD_SET(master_socket, &readfds);
		int max_sd = master_socket;

		int sd = 0;
		socket_select_add_child_sockets_to_set(sd, max_sd, client_socket, readfds);
		//wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
		int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		if ((activity < 0) && (errno != EINTR))
		{
			perror("select");
		}

		socket_select_catch_incoming_conections(master_socket, client_socket, readfds);
		socket_select_check_operations(sd, client_socket, readfds);
	}
	socket_close_conection(master_socket);
}

void *server_handler(void *fd_pointer);

void socket_start_listening_miltithreaded(char* ip, int port)
{
	int master_socket, client_socket;
	if ((master_socket = socket_create_listener(ip, port)) < 0)
	{
		exit(EXIT_FAILURE);
	}

	while((client_socket = socket_accept_conection(master_socket)) != 0)
	{
		pthread_t server_thread;
		int *new_sock = malloc(sizeof(int));
		*new_sock = client_socket;
		pthread_create(&server_thread, NULL, server_handler, (void*)new_sock);
	}

	if(client_socket < 0)
	{
		exit(EXIT_FAILURE);
	}
	socket_close_conection(master_socket);
}

void *server_handler(void *fd_pointer)
{
	printf("Hello Server Handler \n");
	int sock = *(int *) fd_pointer;
	int read_size;
	static char client_message[2000];

	static int send_once = 0;
	if (send_once < 1)
	{
		send_once++;
	}

	while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
	{
		printf("Read Size %d \n", read_size);
		write(sock, client_message, strlen(client_message));
	}
	if (read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if (read_size == -1)
	{
		perror("recv failed");
	}
	free(fd_pointer);

	return 0;
}
