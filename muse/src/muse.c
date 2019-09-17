/*
 ============================================================================
 Name        : muse.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "muse.h"

void muse_server();

int main(void) {
	int response = muse_logger_create();
	if (response < 0)
		return response;

	response = muse_config_load();
	if (response < 0)

	{
		muse_logger_destroy();
		return response;
	}
	muse_server();
	muse_config_free();
	muse_logger_destroy();
	return EXIT_SUCCESS;
}


void muse_server(){
	crearSocket(&muse_socket);
	if (setearParaEscuchar(&muse_socket,muse_config->listen_port)<0){
		muse_logger_error("Error al crear server");
		return;
	}

	muse_logger_info("Esperando conexion del libmuse");

	int libmuse_fd=aceptarConexion(muse_socket);


	if(libmuse_fd==-1){
		muse_logger_error("Error al establecer conexion con el libmuse");
		return;
	}

	muse_logger_info("Conexion establecida con libmuse");
	int bytesRecibidos;
	int protocolo;
	while(1){
		bytesRecibidos=recv(libmuse_fd,&protocolo,sizeof(int),0);

		if(bytesRecibidos<=0){

			muse_logger_error("Error al recibir la operacion del DAM");
			exit(1);
		}
		switch(protocolo){
			case HANDSHAKE:{
				muse_logger_info("Recibi malloc de libmuse");
				break;
			}
			case MALLOC:{
				muse_logger_info("Recibi malloc de libmuse");
				break;
			}
			case FREE:{
				muse_logger_info("Recibi malloc de libmuse");
				break;
			}

		}

	}
}

