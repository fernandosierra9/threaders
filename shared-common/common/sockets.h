/*
 * sockets.h
 *
 *  Created on: 17 sep. 2019
 *      Author: utnso
 */

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
#include <commons/collections/list.h>
#include <commons/string.h>
#include <fcntl.h>
#include <commons/log.h>

//Estructura para manejar el protocolo
typedef enum{
	HANDSHAKE,     		//general
	MALLOC,
	FREE,
	COPY,
	GET
}t_protocolo;




#endif /* COMMON_SOCKETS_H_ */
