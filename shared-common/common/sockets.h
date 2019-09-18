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

#define BACKLOG 20

//Estructura para manejar el protocolo
typedef enum{
	HANDSHAKE,     		//general
	MALLOC,
	FREE_MALLOC,
	COPY,
	GET
}t_protocolo;

typedef struct
{
	uint32_t memoria;
} t_malloc;


int crearSocket(int *mySocket);
int setearParaEscuchar(int *mySocket, int puerto);
int conectar(int* mySocket, int puerto, char *ip) ;
int aceptarConexion(int fd);

void serializarYEnviar(int socket, int tipoDePaquete, void* package);
void* recibirYDeserializar(int socket,int tipo);

#endif /* COMMON_SOCKETS_H_ */
