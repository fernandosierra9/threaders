#include "utils.h"

static int NEW_LINE = '\n';
static int END_LINE = '\0';
static int DOT = '.';
static int SLIDE = '/';
static char* COMMA = ",";
static char* EMPTY_STRING = "";
static char* OPENING_SQUARE_BRACKET = "[";
static char* CLOSING_SQUARE_BRACKET = "]";

void utils_end_string(char *string) {
	if ((strlen(string) > 0) && (string[strlen(string) - 1] == NEW_LINE))
		string[strlen(string) - 1] = END_LINE;
}

bool utils_is_empty(char* string) {
	return string == NULL || string_is_empty(string);
}

char* utils_get_parameter_i(char** array, int i) {
	return array[i] != NULL ? array[i] : EMPTY_STRING;
}

char* utils_get_extension(char* file_name) {
	char *extension = strrchr(file_name, DOT);
	return !extension || extension == file_name ? EMPTY_STRING : extension + 1;
}

char* utils_get_file_name(char* path) {
	char *file = strrchr(path, SLIDE);
	return !file || file == path ? EMPTY_STRING : file + 1;
}

bool utils_is_number(char* string) {
	for (int i = 0; i < strlen(string); i++) {
		if (!isdigit(string[i]))
			return false;
	}
	return strlen(string) != 0;
}

void utils_free_array(char** array) {
	unsigned int i = 0;
	for (; array[i] != NULL; i++) {
		free(array[i]);
	}
	free(array);
}

char* utils_array_to_string(char** array) {
	int i = 0;
	char* aux;
	char* ret = string_new();
	string_append(&ret, OPENING_SQUARE_BRACKET);
	while (array[i] != NULL) {
		aux = array[i];
		string_append(&ret, aux);
		free(aux);
		if (array[i + 1] != NULL) {
			string_append(&ret, COMMA);
		}
		i++;
	}
	string_append(&ret, CLOSING_SQUARE_BRACKET);
	return ret;
}

void utils_delay(int seconds) {
	int millis = 1000 * seconds;
	clock_t start = clock();
	while (clock() < start + millis)
		;
}

void utils_buffer_create(t_package* package) {
	package->buffer = malloc(sizeof(t_buffer));
	package->buffer->size = 0;
	package->buffer->stream = NULL;
}

t_package* utils_package_create(t_protocol code) {
	t_package* package = malloc(sizeof(t_package));

	package->operation_code = code;

	utils_buffer_create(package);

	return package;
}

void utils_package_add(t_package* package, void* value, int size) {
	package->buffer->stream = realloc(package->buffer->stream,
			package->buffer->size + size + sizeof(int));

	memcpy(package->buffer->stream + package->buffer->size, &size, sizeof(int));
	memcpy(package->buffer->stream + package->buffer->size + sizeof(int), value,
			size);
	package->buffer->size += size + sizeof(int);
}

void utils_package_destroy(t_package* package) {
	free(package->buffer->stream);
	free(package->buffer);
	free(package);
}

void utils_package_send_to(t_package* t_package, int client_socket) {
	int bytes = t_package->buffer->size + 2 * sizeof(int);
	void* to_send = serializer_serialize_package(t_package, bytes);

	send(client_socket, to_send, bytes, 0);

	free(to_send);
}

void utils_serialize_and_send(int socket, int protocol, void* package_send) {
	switch (protocol) {
	case HANDSHAKE: {
		break;
	}
	case MALLOC: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_malloc*) package_send)->tam,
				sizeof(uint32_t));
		utils_package_add(package, &((t_malloc*) package_send)->id_libmuse,
				sizeof(int));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}
	case MEMFREE: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_free*) package_send)->dir,
				sizeof(uint32_t));
		utils_package_add(package, &((t_free*) package_send)->self_id,
						sizeof(int));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}
	case GET: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_get*) package_send)->src,
				sizeof(uint32_t));
		utils_package_add(package, &((t_get*) package_send)->id_libmuse,
				sizeof(int));
		utils_package_add(package, &((t_get*) package_send)->size, sizeof(int));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}
	case COPY: {
		printf("\n ****empieza test****");
		int algo;
		printf("\n ****empieza memcpy****");
		memcpy(&algo,((t_copy*) package_send)->content,4);
		printf("\n numero %d",algo);
		printf("\n ****termina memcpy****");
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_copy*) package_send)->dst,
				sizeof(uint32_t));

		utils_package_add(package, &((t_copy*) package_send)->self_id,
				sizeof(int));
		utils_package_add(package, &((t_copy*) package_send)->size, sizeof(int));

		utils_package_add(package, ((t_copy*) package_send)->content,4);

		utils_package_send_to(package, socket);

		utils_package_destroy(package);
		break;
	}
	case MALLOC_OK: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_malloc_ok*) package_send)->ptr,
				sizeof(uint32_t));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}
	case GET_OK: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_get_ok*) package_send)->res,
				sizeof(void*));
		utils_package_add(package, &((t_get_ok*) package_send)->tamres,
				sizeof(int));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}
	case FREE_RESPONSE: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_free_response*) package_send)->res,
				sizeof(int));
		utils_package_send_to(package, socket);
		utils_package_destroy(package);
		break;
	}
	case READ_DIR: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_read_dir*) package_send)->id_sac_cli,sizeof(uint32_t));
		utils_package_add(package, &((t_read_dir*) package_send)->pathname,sizeof(char));
		utils_package_send_to(package,socket);
		utils_package_destroy(package);
		break;
	}
	case READ: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_read*) package_send)->id_sac_cli,sizeof(uint32_t));
		utils_package_add(package, ((t_read*) package_send)->pathname, strlen(((t_read*) package_send)->pathname)+1);
		utils_package_send_to(package,socket);
		utils_package_destroy(package);
		break;
	}
	case OPEN: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_open*) package_send)->id_sac_cli,sizeof(uint32_t));
		utils_package_add(package, &((t_open*) package_send)->pathname, strlen(((t_open*) package_send)->pathname));
		utils_package_send_to(package,socket);
		utils_package_destroy(package);
		break;
	}
	case GET_ATTR: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_get_attr*) package_send)->id_sac_cli,sizeof(uint32_t));
		utils_package_add(package, &((t_get_attr*) package_send)->pathname, strlen(((t_get_attr*) package_send)->pathname));
		utils_package_send_to(package,socket);
		utils_package_destroy(package);
		break;
	}
	case CREATE_DIR: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_create_directory*) package_send)->id_sac_cli,sizeof(uint32_t));
		utils_package_add(package, &((t_create_directory*) package_send)->pathname, strlen(((t_create_directory*) package_send)->pathname));
		utils_package_send_to(package,socket);
		utils_package_destroy(package);
		break;
	}
	case WRITE: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_write*) package_send)->id_sac_cli,sizeof(uint32_t));
		utils_package_add(package, &((t_write*) package_send)->pathname, strlen(((t_write*) package_send)->pathname));
		utils_package_send_to(package,socket);
		utils_package_destroy(package);
		break;
	}
	case RM_DIR: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_rm_directory*) package_send)->id_sac_cli,sizeof(uint32_t));
		utils_package_add(package, &((t_rm_directory*) package_send)->pathname, strlen(((t_rm_directory*) package_send)->pathname));
		utils_package_send_to(package,socket);
		utils_package_destroy(package);
		break;
	}
	case MK_DIR: {
		t_package* package = utils_package_create(protocol);
		utils_package_add(package, &((t_mk_directory*) package_send)->id_sac_cli,sizeof(uint32_t));
		utils_package_add(package, &((t_mk_directory*) package_send)->pathname, strlen(((t_mk_directory*) package_send)->pathname));
		utils_package_send_to(package,socket);
		utils_package_destroy(package);
		break;
	}
  }
}

void* utils_receive_and_deserialize(int socket, int package_type)
{
	void iterator(t_buffer* value) {
		printf("%d \n", value->size);
		int dest;
		memcpy(&dest, value->stream, value->size);
		printf("%d \n", dest);
	}
	switch (package_type) {
	case MALLOC: {
		t_malloc *malloc_request = malloc(sizeof(t_malloc));
		//Recibe los parametros en una lista
		t_list* list = utils_receive_package(socket);
		//Muestra el resultado obtenido
		list_iterate(list, (void*) iterator);
		//obtiene y guarda en un puntero desde un nodo de la lista dado un index
		utils_get_from_list_to(&malloc_request->tam, list, 0);
		utils_get_from_list_to(&malloc_request->id_libmuse, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return malloc_request;
	}
	case MEMFREE: {
		t_free *free_request = malloc(sizeof(t_free));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&free_request->dir, list, 0);
		utils_get_from_list_to(&free_request->self_id, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return free_request;
	}
	case GET: {
		t_get *get_request = malloc(sizeof(t_get));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->src, list, 0);
		utils_get_from_list_to(&get_request->id_libmuse, list, 1);
		utils_get_from_list_to(&get_request->size, list, 2);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
	case COPY: {

		t_copy *copy_req = malloc(sizeof(t_copy));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&copy_req->dst, list, 0);
		utils_get_from_list_to(&copy_req->self_id, list, 1);
		utils_get_from_list_to(&copy_req->size, list, 2);
		//falta recervar memoria parar content

		copy_req->content = malloc(copy_req->size);
		utils_get_from_list_to(copy_req->content, list, 3);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);

		return copy_req;
	}
	case MALLOC_OK: {
		t_malloc_ok* malloc_recv = malloc(sizeof(t_malloc_ok));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&malloc_recv->ptr, list, 0);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return malloc_recv;
	}
	case FREE_RESPONSE: {
		t_free_response* free_recv = malloc(sizeof(t_free_response));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&free_recv->res, list, 0);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return free_recv;
	}
	case GET_OK: {
		t_get_ok* get_recv = malloc(sizeof(t_get_ok));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_recv->res, list, 0);
		utils_get_from_list_to(&get_recv->tamres, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_recv;
	}
	case GET_ATTR: {
		t_get_attr *get_request = malloc(sizeof(t_get_attr));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->id_sac_cli,list,0);
		utils_get_from_list_to(&get_request->pathname, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
	case READ_DIR: {
		t_read_dir *get_request = malloc(sizeof(t_read_dir));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->id_sac_cli,list,0);
		utils_get_from_list_to(&get_request->pathname, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
	case READ: {
		t_read *get_request = malloc(sizeof(t_read));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->id_sac_cli,list,0);
		utils_get_from_list_to(get_request->pathname, list, 1);
		char *test ;
		test = malloc(strlen(get_request->pathname)+1);
		test = strdup(get_request->pathname);
		printf("test de path: %s \n",get_request->pathname);
		get_request->pathname = test;
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
	case OPEN: {
		t_open *get_request = malloc(sizeof(t_open));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->id_sac_cli,list,0);
		utils_get_from_list_to(&get_request->pathname, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
	case MK_DIR: {
		t_mk_directory *get_request = malloc(sizeof(t_mk_directory));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->id_sac_cli,list,0);
		utils_get_from_list_to(&get_request->pathname, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
	case CREATE_DIR: {
		t_create_directory *get_request = malloc(sizeof(t_create_directory));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->id_sac_cli,list,0);
		utils_get_from_list_to(&get_request->pathname, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
	case WRITE: {
		t_write *get_request = malloc(sizeof(t_write));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->id_sac_cli,list,0);
		utils_get_from_list_to(&get_request->pathname, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
	case RM_DIR: {
		t_rm_directory *get_request = malloc(sizeof(t_rm_directory));
		t_list* list = utils_receive_package(socket);
		utils_get_from_list_to(&get_request->id_sac_cli,list,0);
		utils_get_from_list_to(&get_request->pathname, list, 1);
		list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
		return get_request;
	}
  }
	return NULL;
}

static void utils_destroy_list(t_buffer *self) {
	free(self->stream);
	free(self);
}

void utils_get_from_list_to(void *parameter, t_list *list, int index) {
	t_buffer *buffer;
	buffer = list_get(list, index);
	memcpy(parameter, buffer->stream, buffer->size);
}

void* utils_receive_buffer(int* size, int socket_cliente) {
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

t_list* utils_receive_package(int socket_cliente) {
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = utils_receive_buffer(&size, socket_cliente);

	while (desplazamiento < size) {
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		t_buffer* valor = malloc(sizeof(t_buffer));
		valor->stream = malloc(tamanio);
		valor->size = tamanio;
		memcpy(valor->stream, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}
