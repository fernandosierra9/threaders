#include "sockets.h"

int crearSocket(int *mySocket) {
	int opcion=1;

	if ((*mySocket=socket(AF_INET, SOCK_STREAM,0))==-1){
		perror("-1 al crear el socket");
		return -1;
	}
	if (setsockopt(*mySocket, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(int))==-1){
		perror("-1 al setear las opciones del socket");
		return -1;
	}
	return 0;
}

int setearParaEscuchar(int *mySocket, int puerto) {
	struct addrinfo direccion, *infoBind = malloc(sizeof(struct addrinfo));

	memset(&direccion, 0, sizeof direccion);
	direccion.ai_family = AF_INET;
	direccion.ai_socktype = SOCK_STREAM;
	direccion.ai_flags = AI_PASSIVE;

	char *port= malloc(sizeof(char)*6);

	port = string_itoa(puerto);
	getaddrinfo(NULL, port, &direccion, &infoBind);

	free(port);

	if(bind(*mySocket,infoBind->ai_addr, infoBind->ai_addrlen)==-1){
		perror("Fallo el bind");
		free(infoBind);
		return -1;
	}
	//ver si hay que sacar el free
	free(infoBind);
	if (listen(*mySocket, BACKLOG) ==-1){
		perror("Fallo el listen");
		return -1;
	}
	//Si en todos los procesos llamamos al logger con el mismo nombre podemos llamarlo desde la biblioteca
	//log_info(logger, " ... Escuchando conexiones ... ");
	return 0;
}

int conectar(int* mySocket, int puerto, char *ip) {
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	char *stringPort = string_itoa(puerto);
	getaddrinfo(ip, stringPort, &hints, &res);
	free(stringPort);
	if(connect(*mySocket, res->ai_addr, res->ai_addrlen)!=0){
		return -1;
	}
	return 0;
}

int aceptarConexion(int fd){

	struct sockaddr_in cliente;

	unsigned int len = sizeof(struct sockaddr);

	return accept(fd,(void*)&cliente,&len);

}


int escuchar(int socketListener, fd_set *fd,  void *(funcionSocketNuevo)(int, void*), void *argumentosSocketNuevo,
				void *(funcionSocketExistente)(int, void*), void *argumentosSocketExistente){

	fd_set copia = *fd;
	int socketsComunicandose=0;
	if((socketsComunicandose=select(FD_SETSIZE,&copia,NULL,NULL,NULL))==-1) {

		perror("Fallo en el select");
		return -1;
	}

	if(FD_ISSET(socketListener,&copia)) {

		int socketNuevo=0;
		if ((socketNuevo = accept(socketListener, NULL, 0)) < 0) {

			perror("Error al aceptar conexion entrante");
			return -1;
		} else {

			FD_SET(socketNuevo,fd);

			if(funcionSocketNuevo!=NULL) {
				funcionSocketNuevo(socketNuevo, argumentosSocketNuevo);
			}
		}
	}else{

		int i;
		for (i = 0; i < FD_SETSIZE; i++) {

			if (FD_ISSET(i, &copia) && i != socketListener) {
				funcionSocketExistente(i, argumentosSocketExistente);
			}
		}
	}
	return 0;
}

void serializarYEnviar(int socket, int tipoDepackage, void* package){

	switch(tipoDepackage){
		case HANDSHAKE:{
			break;
		}
		case MALLOC:{
			t_package* package = malloc(sizeof(t_package));
			package->operation_code = MALLOC;
			package->buffer = malloc(sizeof(t_buffer));
			package->buffer->size = sizeof(uint32_t);
			package->buffer->stream = malloc(package->buffer->size);
			memcpy(package->buffer->stream, &((t_malloc*)package)->memoria, package->buffer->size);
			int bytes = package->buffer->size + 2*sizeof(int);
     		void* a_enviar = serializer_serialize_package(package, bytes);
			send(socket, a_enviar, bytes, 0);
			free(a_enviar);
			break;
		}

	}

}

void* recibirYDeserializar(int socket,int tipo){
	switch(tipo){
		case MALLOC:
		{
			t_malloc *pedido_malloc=malloc (sizeof(t_malloc));
			int size;
			recv(socket, &size, sizeof(int), MSG_WAITALL);
			recv(socket, &pedido_malloc->memoria, size, MSG_WAITALL);
			return pedido_malloc;
		}
	}
	return NULL;
}


