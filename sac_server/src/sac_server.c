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
			case GET_ATTR: {
 				t_get_attr *get_attr_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi GET_ATTR de SAC_CLI, Path: %s", get_attr_dir->pathname);
				t_protocol get_attr_protocol_response = GET_ATTR_RESPONSE;
				int res;
				t_get_attr_server *get_attr_send_server = malloc(sizeof(t_get_attr_server));
 				struct sac_file_t* node = sac_server_getattr(get_attr_dir->pathname, &res);
				sac_server_logger_info("GET_ATTR Response: %d", res);
				if (res == 0) {
					get_attr_send_server->state = node->state;
					get_attr_send_server->file_size = node->file_size;
					get_attr_send_server->creation_date = node->creation_date;
					get_attr_send_server->modified_date = node->modified_date;
					utils_serialize_and_send(fd, get_attr_protocol_response, get_attr_send_server);
				} 
				if (res < 0) {
					send(fd, &res, sizeof(int), 0);
				}
				break;
			}
			case READ_DIR: {
				t_read_dir *read_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi READ_DIR de SAC_CLI, Path: %s", read_dir->pathname);
				t_protocol read_dir_protocol_response = READ_DIR_RESPONSE;
				t_list* nodes = list_create();
				t_read_dir_server *read_dir_response_server = malloc(sizeof(t_read_dir_server));
				int res = sac_server_readdir(read_dir->pathname, nodes);
				sac_server_logger_info("READ_DIR Response: %d", res);
				read_dir_response_server->nodes = nodes;
				read_dir_response_server->res = res;
				utils_serialize_and_send(fd, read_dir_protocol_response, read_dir_response_server);
				break;
			}
			case READ: {
				t_read *read_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi READ de SAC_CLI, Path: %s", read_dir->pathname);
				t_protocol read_response_protocol = READ_RESPONSE;
				t_read_server *read_response_server = malloc(sizeof(t_read_server));
				char* buffer = malloc(read_dir->size);
				size_t size = read_dir->size;
				off_t offset = read_dir->offset;
				int read_res = sac_server_read(read_dir->pathname, &buffer, &size, &offset);
				
				if (read_res > 0) {
					read_response_server->buf = strdup(buffer);
					read_response_server->response = read_res;
					read_response_server->offset = offset;
					read_response_server->size = size;
					utils_serialize_and_send(fd, read_response_protocol, read_response_server);
				}

				if (read_res < 0 || read_res == 0) {
					send(fd, &read_res, sizeof(int), 0);
					sac_server_logger_info("NEGATIVE OR CERO RESPONSE");
				}
				break;
			}
			case WRITE: {
				t_write *write_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi WRITE de SAC_CLI, Path: %s", write_dir->pathname);
				t_protocol write_response_protocol = WRITE_RESPONSE;
				t_write_server *write_response_server = malloc(sizeof(t_write_server));
				int res = sac_server_write(write_dir->pathname, write_dir->buf, write_dir->size, write_dir->offset);
				//int res = sac_server_write(write_dir->pathname, &write_dir->buf, &write_dir->size, &write_offset);
				sac_server_logger_info("WRITE Response: %d", res);
				
				if (res == 0) {
					write_response_server->response = res;
					write_response_server->buf = write_dir->buf;
					write_response_server->size = write_dir->size;
					write_response_server->offset = write_dir->offset;
					utils_serialize_and_send(fd, write_response_protocol, write_response_server);
				}

				if (res < 0) {
					send(fd, &res, sizeof(int), 0);
				}
				break;
			}
			case MK_DIR: {
				t_mk_directory *mk_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi MK_DIR de SAC_CLI, Path: %s", mk_dir->pathname);
				int res = sac_server_create_directory(mk_dir->pathname);
				sac_server_logger_info("MK_DIR Response: %d", res);
				send(fd, &res, sizeof(int), 0);
				break;
			}
			case RM_DIR: {
				t_read_dir *rm_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi RM_DIR de SAC_CLI, Path: %s", rm_dir->pathname);
				int res = sac_server_remove_directory(rm_dir->pathname);
				sac_server_logger_info("RM_DIR Response: %d", res);
				send(fd, &res, sizeof(int), 0);
				break;
			}
			case FLUSH: {
				t_flush *flush_dir = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi FLUSH de SAC_CLI, Path: %s", flush_dir->pathname);
				int res = sac_server_flush();
				sac_server_logger_info("FLUSH Response: %d", res);
				send(fd, &res, sizeof(int), 0);
				break;
			}
			case UNLINK_NODE: {
				t_unlink_node *unlink_node = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi UNLINK de SAC_CLI, Path: %s", unlink_node->pathname);
				int res = sac_server_unlink_node(unlink_node->pathname);
				sac_server_logger_info("UNLINK Response: %d", res);
				send(fd, &res, sizeof(int), 0);
				break;
			}
			case MK_NODE: {
				t_unlink_node *make_node = utils_receive_and_deserialize(fd, protocol);
				sac_server_logger_info("Recibi MAKE NODE de SAC_CLI, Path: %s", make_node->pathname);
				int res = sac_server_make_node(make_node->pathname);
				sac_server_logger_info("MK_NODE Response: %d", res);
				send(fd, &res, sizeof(int), 0);
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

	mlock(bitmap_start, BITMAP_BLOCK_SIZE * BLOCK_SIZE);
	mlock(node_table_start, NODE_TABLE_SIZE * BLOCK_SIZE);
	madvise(header_start, ACTUAL_DISC_SIZE_B , MADV_RANDOM);

	obtain_free_blocks();
}

static void close_administrative_structures() {
	sac_server_logger_info("CLOSING ADMINISTRATIVE STRUCTURES");


	fdatasync(discDescriptor);
	
	munlockall(); /* Desbloquea todas las paginas que tenia bloqueadas */

	if (munmap(header_start, ACTUAL_DISC_SIZE_B ) == -1) printf("ERROR");

}

static void exit_gracefully(int status) {
	exit(status);
}