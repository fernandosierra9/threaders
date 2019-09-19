#ifndef COMMON_SOCKETS_H_
#define COMMON_SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <commons/string.h>
#include "protocols.h"

#define BACKLOG 100

/**
 * @NAME: socket_create_listener
 * @DESC: Creo un socket de escucha y lo devuelvo, o -1 se hubo error. Me pongo a escuchar.
 * @PARAMS:
 * 		ip   - ip del server. Si es NULL, usa el localhost: 127.0.0.1
 * 		port - puerto en el que escuchar
 */
int socket_create_listener(char* ip, int port);

/**
 * @NAME: socket_connect_to_server
 * @DESC: Me conecto al server, y devuelvo el socket, o -1 si hubo error
 */
int socket_connect_to_server(char* ip, int port, int server_socket);

/**
 * @NAME: socket_accept_conection
 * @DESC: Acepta una nueva conexión y devuelve el nuevo socket conectado
 */
int socket_accept_conection(int server_socket);

/**
 * @NAME: socket_get_ip
 * @DESC: Devuelve la IP de un socket
 */
char* socket_get_ip(int fd);

#endif /* COMMON_SOCKETS_H_ */
