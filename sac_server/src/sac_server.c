#include "sac_server.h"


/* Local functions */
static void init_server(int port);
static void *handle_connection(void *arg);
static void exit_gracefully(int status);
static void init_administrative_structures(char *argv[]);
static void close_administrative_structures();

int main(int argc, char *argv[]) {

	sac_server_logger_create();
	sac_server_config_load();

	init_administrative_structures(argv);
	init_server(sac_server_get_listen_port());
	close_administrative_structures();

	sac_server_logger_destroy();
	exit_gracefully(EXIT_FAILURE);
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
		t_list* nodes = list_create();
		int res = sac_server_readdir("/", nodes);
		for (int j=0; j<list_size(nodes); j++) {
			printf("\n NODO: %s \n", (char*) list_get(nodes, j));
		}
	}

		struct sockaddr_in client_info;
		socklen_t addrlen = sizeof client_info;

		pthread_attr_t attrs;
		pthread_attr_init(&attrs);
		pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);

		for (;;) {
			int *accepted_fd = malloc(sizeof(int));
			*accepted_fd = accept(sac_server_socket, (struct sockaddr *) &client_info, &addrlen);

			sac_server_logger_info("Creando un hilo para atender una conexión en el socket %d", *accepted_fd);

			pthread_t tid;
			pthread_create(&tid, &attrs, handle_connection, accepted_fd);
		}

		pthread_attr_destroy(&attrs);
		close(sac_server_socket); 
	}
	
}

static void *handle_connection(void *arg) {
	int received_bytes;
	int protocol;

	int fd = *((int *)arg);
	free(arg);

	for (;;) {
		received_bytes = recv(fd, &protocol, sizeof(int), 0);

		if (received_bytes <= 0){
			sac_server_logger_error("Error al recibir la operacion del SAC_CLI");
			sac_server_logger_error("Se desconecto SAC_CLI");
			break;
		}

		switch (protocol){
			case READ_DIR: {
				sac_server_logger_info("Recibi READ_DIR de SAC_CLI");
				t_read_dir *read_dir = utils_receive_and_deserialize(fd, protocol);
				t_protocol read_dir_protocol_response = READ_DIR_RESPONSE;
				t_list* nodes = list_create();
				t_read_dir_server *read_dir_send_server = malloc(sizeof(t_read_dir_server));

				//int res = sac_server_readdir(read_dir->pathname, &nodes);
				int res = sac_server_readdir("/", nodes);
				sac_server_readdir("/", nodes);
				read_dir_send_server->nodes = nodes;
				utils_serialize_and_send(fd, read_dir_protocol_response, read_dir_send_server);

				break;
			}
			case GET_ATTR: {
				sac_server_logger_info("Recibi GET_ATTR de SAC_CLI");
				t_get_attr *get_attr_dir = utils_receive_and_deserialize(fd, protocol);
				t_protocol get_attr_protocol_response = GET_ATTR_RESPONSE;
				struct sac_file_t node;
				t_get_attr_server *get_attr_send_server = malloc(sizeof(t_get_attr_server));

				//int res = sac_server_getattr(get_attr_send_server->pathname, &node);
				int res = sac_server_getattr("/", &node);
				get_attr_send_server->state = node.state;
				get_attr_send_server->file_size = node.file_size;
				get_attr_send_server->creation_date = node.creation_date;
				get_attr_send_server->modified_date = node.modified_date;
				utils_serialize_and_send(fd, get_attr_protocol_response, get_attr_send_server);
				break;
			}
			case READ: {
				sac_server_logger_info("Recibi READ de SAC_CLI");
				t_read *read_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("PATHNAME: %s", read_dir->pathname);
				sac_server_logger_info("ID_SAC_CLI: %d", read_dir->id_sac_cli);
				break;
			}
			case OPEN: {
				sac_server_logger_info("Recibi OPEN_DIR de SAC_CLI");
/* 				t_open *open_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("PATHNAME: %s", open_dir->pathname);
				sac_server_logger_info("ID_SAC_CLI: %d", open_dir->id_sac_cli); */
				break;
			}
			case MK_DIR: {
				sac_server_logger_info("Recibi MK_DIR de SAC_CLI");
				t_mk_directory *mk_dir = utils_receive_and_deserialize(fd, protocol);
				int res = sac_server_create_directory("/testt");
				sac_server_logger_info("RES: %d", res);
				send(fd, &res, sizeof(int), 0);
				break;
			}
			case WRITE: {
				sac_server_logger_info("Recibi WRITE_DIR de SAC_CLI");
				t_write *write_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("PATHNAME: %s", write_dir->pathname);
				sac_server_logger_info("ID_SAC_CLI: %d", write_dir->id_sac_cli);
				break;
			}
			case RM_DIR: {
				sac_server_logger_info("Recibi RM_DIR de SAC_CLI");
				t_read_dir *rm_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("PATHNAME: %s", rm_dir->pathname);
				sac_server_logger_info("ID_SAC_CLI: %d", rm_dir->id_sac_cli);
				break;
			}
		}
	}
}

static void init_administrative_structures(char *argv[]) {
	int fd;
		// Setea el path del disco
	if (argv[1] != NULL){
		fuse_disc_path = malloc(strlen(argv[1]));
		strcpy(fuse_disc_path, argv[1]);
	} else {
		printf("Disc not specified: Unloading modules.");
		exit(0);
	}
	sac_server_logger_info("INITIALIZING ADMINISTRATIVE STRUCTURES");
	sac_server_logger_info("DISC PATH: %s", fuse_disc_path);
	// Obiene el tamanio del disco
	fuse_disc_size = path_size(DISC_PATH);
	sac_server_logger_info("DISC SIZE %d", fuse_disc_size);
	_bitarray_64 = get_size() / 64;
	sac_server_logger_info("BIT ARRAY SIZE %d", _bitarray_64);
	_bitarray_64_leak = get_size() - (_bitarray_64 * 64);
	sac_server_logger_info("BIT ARRAY leak %d", _bitarray_64);

	if ((discDescriptor = fd = open(DISC_PATH, O_RDWR, 0)) == -1) {
		sac_server_logger_error("Error");
	}
	
	header_start = (struct sac_header_t*) mmap(NULL, ACTUAL_DISC_SIZE_B , PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
	header_data = *header_start;
	bitmap_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS];
	node_table_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS + BITMAP_BLOCK_SIZE];
	data_block_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS + BITMAP_BLOCK_SIZE + NODE_TABLE_SIZE];
}

static void close_administrative_structures() {
	sac_server_logger_info("CLOSING ADMINISTRATIVE STRUCTURES");
	mlock(bitmap_start, BITMAP_BLOCK_SIZE * BLOCK_SIZE);
	mlock(node_table_start, NODE_TABLE_SIZE * BLOCK_SIZE);
	madvise(header_start, ACTUAL_DISC_SIZE_B , MADV_RANDOM);

	fdatasync(discDescriptor);
	
	munlockall(); /* Desbloquea todas las paginas que tenia bloqueadas */

	if (munmap(header_start, ACTUAL_DISC_SIZE_B ) == -1) printf("ERROR");

}

static void exit_gracefully(int status) {
	exit(status);
}