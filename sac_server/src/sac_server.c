#include "sac_server.h"


/* Local functions */
static void init_server(int port);
static void *handle_connection(void *arg);
static void exit_gracefully(int status);

int main(int argc, char *argv[]) {

	sac_server_logger_create();
	sac_server_config_load();

	init_server(sac_server_get_listen_port());

	sac_server_logger_destroy();
	exit_gracefully(EXIT_FAILURE);
	
	//int fd;
	// Obiene el tamanio del disco
	//fuse_disc_size = path_size(DISC_PATH);

	/*
	BitArray
	*/

/* 	if ((discDescriptor = fd = open(DISC_PATH, O_RDWR, 0)) == -1) {
		sac_server_logger_error("Error");
	}
	
	header_start = (struct sac_header_t*) mmap(NULL, ACTUAL_DISC_SIZE_B , PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
	header_data = *header_start;
	bitmap_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS];
	node_table_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS + BITMAP_BLOCK_SIZE];
	data_block_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS + BITMAP_BLOCK_SIZE + NODE_TABLE_SIZE];
	
	mlock(bitmap_start, BITMAP_BLOCK_SIZE * BLOCK_SIZE);
	mlock(node_table_start, NODE_TABLE_SIZE * BLOCK_SIZE);
	madvise(header_start, ACTUAL_DISC_SIZE_B , MADV_RANDOM); */

	//fdatasync(discDescriptor);
	
	//munlockall(); /* Desbloquea todas las paginas que tenia bloqueadas */

	//if (munmap(header_start, ACTUAL_DISC_SIZE_B ) == -1) printf("ERROR");

	//close(fd);
}

static void init_server(int port) {
	
	int sac_server_socket = socket_create_listener("127.0.0.1", port);

	sac_server_logger_info("Esperando conexion de SAC_CLI: %d", sac_server_socket);

	if (sac_server_socket < 0) {
		switch (sac_server_socket) {
		case NO_FD_ERROR:
			sac_server_logger_error("No hay file descriptor disponible para el listener.");
			break;
		case BIND_ERROR:
			sac_server_logger_error("Error al intentar bindear.");
			break;
		case LISTEN_ERROR:
			sac_server_logger_error("Error al intentar crear cola de escucha.");
			break;
		}
		exit_gracefully(EXIT_FAILURE);
	} else {
		sac_server_logger_info("Escuchando en el socket %d", sac_server_socket);

		struct sockaddr_in client_info;
		socklen_t addrlen = sizeof client_info;

		pthread_attr_t attrs;
		pthread_attr_init(&attrs);
		pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);

		for (;;) {
			int *accepted_fd = malloc(sizeof(int));
			*accepted_fd = accept(sac_server_socket, (struct sockaddr *) &client_info, &addrlen);

			sac_server_logger_info("Creando un hilo para atender una conexión en el socket %d", *accepted_fd);

			pthread_t tid;
			pthread_create(&tid, &attrs, handle_connection, accepted_fd);
		}

		pthread_attr_destroy(&attrs);
	}
}

static void *handle_connection(void *arg) {
	int received_bytes;
	int protocol;

	int fd = *((int *)arg);
	free(arg);

	received_bytes = recv(fd, &protocol, sizeof(int), 0);

	if (received_bytes <= 0){
		sac_server_logger_error("Error al recibir la operacion del SAC_CLI");
		sac_server_logger_error("Se desconecto SAC_CLI");
		exit(1);
	}

	switch (protocol){
		case READ_DIR: {
			sac_server_logger_info("Recibi OPEN de SAC_CLI");
			t_malloc *malloc_recive = utils_receive_and_deserialize(fd, protocol);
			sac_server_logger_info("MALLOC_TAM: %d", malloc_recive->memoria);
			sac_server_logger_info("ID_LIBMUSE: %d", malloc_recive->id_libmuse);
			break;
		}
	}
}

static void exit_gracefully(int status) {
	exit(status);
}