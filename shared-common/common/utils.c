#include "utils.h"

static int NEW_LINE = '\n';
static int END_LINE = '\0';
static int DOT = '.';
static int SLIDE = '/';
static char* COMMA = ",";
static char* EMPTY_STRING = "";
static char* OPENING_SQUARE_BRACKET = "[";
static char* CLOSING_SQUARE_BRACKET = "]";

void utils_end_string(char *string)
{
	if ((strlen(string) > 0) && (string[strlen(string) - 1] == NEW_LINE))
		string[strlen(string) - 1] = END_LINE;
}

bool utils_is_empty(char* string)
{
	return string == NULL || string_is_empty(string);
}

char* utils_get_parameter_i(char** array, int i)
{
	return array[i] != NULL ? array[i] : EMPTY_STRING;
}

char* utils_get_extension(char* file_name)
{
	char *extension = strrchr(file_name, DOT);
	return !extension || extension == file_name ? EMPTY_STRING : extension + 1;
}

char* utils_get_file_name(char* path)
{
	char *file = strrchr(path, SLIDE);
	return !file || file == path ? EMPTY_STRING : file + 1;
}

bool utils_is_number(char* string)
{
	for (int i = 0; i < strlen(string); i++)
	{
		if (!isdigit(string[i]))
			return false;
	}
	return strlen(string) != 0;
}

void utils_free_array(char** array)
{
	unsigned int i = 0;
	for (; array[i] != NULL; i++)
	{
		free(array[i]);
	}
	free(array);
}

char* utils_array_to_string(char** array)
{
	int i = 0;
	char* aux;
	char* ret = string_new();
	string_append(&ret, OPENING_SQUARE_BRACKET);
	while (array[i] != NULL)
	{
		aux = array[i];
		string_append(&ret, aux);
		free(aux);
		if (array[i + 1] != NULL)
		{
			string_append(&ret, COMMA);
		}
		i++;
	}
	string_append(&ret, CLOSING_SQUARE_BRACKET);
	return ret;
}

void utils_delay(int seconds)
{
	int millis = 1000 * seconds;
	clock_t start = clock();
	while (clock() < start + millis)
		;
}

void utils_buffer_create(t_package* package)
{
	package->buffer = malloc(sizeof(t_buffer));
	package->buffer->size = 0;
	package->buffer->stream = NULL;
}

t_package* utils_package_create(t_protocol code)
{
	t_package* package = malloc(sizeof(t_package));

	package->operation_code =code;

	utils_buffer_create(package);

	return package;
}

void utils_package_add(t_package* package, void* value, int size)
{
	package->buffer->stream = realloc(package->buffer->stream, package->buffer->size + size + sizeof(int));

	memcpy(package->buffer->stream + package->buffer->size, &size, sizeof(int));
	memcpy(package->buffer->stream + package->buffer->size + sizeof(int), value, size);
	package->buffer->size += size + sizeof(int);
}

void utils_package_destroy(t_package* package)
{
	free(package->buffer->stream);
	free(package->buffer);
	free(package);
}

void utils_package_send_to(t_package* t_package, int client_socket)
{
	int bytes = t_package->buffer->size + 2 * sizeof(int);
	void* to_send = serializer_serialize_package(t_package, bytes);

	send(client_socket, to_send, bytes, 0);

	free(to_send);
}

void utils_serialize_and_send(int socket, int protocol, void* package_send)
{
	switch (protocol)
	{
		case HANDSHAKE:
		{
			break;
		}
		case MALLOC:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_malloc*) package_send)->tam,sizeof(uint32_t));
			// printf("\n enviado tamaño %d:",sizeof(uint32_t));
			// printf("\n enviado %d:",((t_malloc*) package_recv)->tam);
			// printf("\n tamaño del paqute a enviar %d:",package->buffer->size);
			utils_package_add(package, &((t_malloc*) package_send)->id_libmuse,sizeof(int));
			// printf("\n enviado tamaño %d:",sizeof(int));
			// printf("\n enviado %d:",((t_malloc*) package_recv)->id_libmuse);
			// printf("\n tamaño del paqute a enviar %d:",package->buffer->size);
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case FREE_MALLOC:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_malloc*) package_send)->tam,sizeof(uint32_t));
			utils_package_add(package, &((t_malloc*) package_send)->id_libmuse,sizeof(int));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case GET:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_get*) package_send)->src,sizeof(uint32_t));
			utils_package_add(package, &((t_get*) package_send)->id_libmuse,sizeof(int));
			utils_package_add(package, &((t_get*) package_send)->size,sizeof(int));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
		case MALLOC_OK:
		{
			t_package* package = utils_package_create(protocol);
			utils_package_add(package, &((t_malloc_ok*) package_send)->ptr,sizeof(uint32_t));
			utils_package_send_to(package, socket);
			utils_package_destroy(package);
			break;
		}
	}
}

void* utils_receive_and_deserialize(int socket, int package_type)
{
	void iterator(t_buffer* value)
	{
		printf("%d \n", value->size);
		int recivido;
		memcpy(&recivido,value->stream,value->size);
		printf("%d \n", recivido);
	}
	switch (package_type)
	{
		case MALLOC:
		{
			t_malloc *malloc_request = malloc(sizeof(t_malloc));
			//Recibe los parametros en una lista
			t_list* list = utils_receive_package(socket);
			//Muestra el resultado obtenido
			list_iterate(list, (void*) iterator);
			//obtiene y guarda en un puntero desde un nodo de la lista dado un index
			utils_get_from_list_to(&malloc_request->tam,list,0);
			utils_get_from_list_to(&malloc_request->id_libmuse,list,1);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return malloc_request;
		}
		case FREE_MALLOC:
		{
			        t_malloc *free_request = malloc(sizeof(t_malloc));
		            t_list* list = utils_receive_package(socket);
					utils_get_from_list_to(&free_request->tam,list,0);
					utils_get_from_list_to(&free_request->id_libmuse,list,1);
					list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
					return free_request;


		}
		case GET:
				{
					        t_get *get_request = malloc(sizeof(t_get));
				            t_list* list = utils_receive_package(socket);
							utils_get_from_list_to(&get_request->src,list,0);
							utils_get_from_list_to(&get_request->id_libmuse,list,1);
							utils_get_from_list_to(&get_request->size,list,2);
							list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
							return get_request;
				}
		case MALLOC_OK:
		{
			t_malloc_ok* malloc_recv = malloc(sizeof(t_malloc_ok));
			t_list* list = utils_receive_package(socket);
			utils_get_from_list_to(&malloc_recv->ptr,list,0);
			list_destroy_and_destroy_elements(list, (void*) utils_destroy_list);
			return malloc_recv;
		}
	}
	return NULL;
}


static void utils_destroy_list(t_buffer *self) {
    free(self->stream);
    free(self);
}

void utils_get_from_list_to(void *parameter,t_list *list,int index){
	t_buffer *buffer; buffer=list_get(list,index);
	memcpy(parameter,buffer->stream,buffer->size);
}

void* utils_receive_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

t_list* utils_receive_package(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = utils_receive_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		t_buffer* valor = malloc(sizeof(t_buffer));
		valor->stream = malloc(tamanio);
		valor->size = tamanio;
		memcpy(valor->stream, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}
