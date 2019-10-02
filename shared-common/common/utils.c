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

void utils_serialize_and_send(int socket, int package_type, void* package_recv)
{
	switch (package_type)
	{
		case HANDSHAKE:
		{
			break;
		}
		case MALLOC:
		{
			t_package* package = utils_package_create(package_type);
			utils_package_add(package, &((t_malloc*) package_recv)->memoria,sizeof(uint32_t));
			printf("\n enviado tamaño %d:",sizeof(uint32_t));
			printf("\n enviado %d:",((t_malloc*) package_recv)->memoria);
			printf("\n tamaño del paqute a enviar %d:",package->buffer->size);
			utils_package_add(package, &((t_malloc*) package_recv)->id_libmuse,sizeof(int));
			printf("\n enviado tamaño %d:",sizeof(int));
			printf("\n enviado %d:",((t_malloc*) package_recv)->id_libmuse);
			printf("\n tamaño del paqute a enviar %d:",package->buffer->size);
			utils_package_send_to(package,socket);
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

			int size;

			/*
			recv(socket, &size, sizeof(int), MSG_WAITALL);
			printf("\n tamaño total size recibido %d:",size);

			recv(socket,&size ,sizeof(int), MSG_WAITALL);
			printf("\n primer tamaño de size de memoria recibido %d:",size);

			recv(socket, &malloc_request->memoria, sizeof(int), MSG_WAITALL);
			printf("\n tamaño de memoria %d:",malloc_request->memoria);


			recv(socket, &size, sizeof(int), MSG_WAITALL);
			printf("\n tamaño de id_lizemuse recibido %d:",size);


			recv(socket, &malloc_request->id_libmuse, sizeof(int), MSG_WAITALL);
			printf("\n libmuse id %d:",malloc_request->id_libmuse);

		    //recv(socket, &size, sizeof(int), MSG_WAITALL);
			//printf("\n algo que quedo recibido %d: ",size);
			printf("\n");
			 */
			t_list* list = utils_receive_package(socket);
			list_iterate(list, (void*) iterator);
			t_buffer *buffer;
			buffer=list_get(list,0);
			memcpy(&malloc_request->memoria,buffer->stream,buffer->size);
			buffer=list_get(list,1);
			memcpy(&malloc_request->id_libmuse,buffer->stream,buffer->size);
			return malloc_request;

		}
	}
	return NULL;
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
