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
		if (array[i + 1] != NULL)
		{
			string_append(&ret, COMMA);
		}
		i++;
	}
	free(aux);
	string_append(&ret, CLOSING_SQUARE_BRACKET);
	return ret;
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

void utils_serialize_and_send(int socket, int package_type, void* package)
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
			utils_package_add(package, &((t_malloc*) package)->memoria,sizeof(t_buffer));
			utils_package_send_to(package,socket);
			utils_package_destroy(package);
			break;
		}
	}
}

void* utils_receive_and_deserialize(int socket, int package_type)
{
	switch (package_type)
	{
		case MALLOC:
		{
			t_malloc *malloc_request = malloc(sizeof(t_malloc));

			int size;
			recv(socket, &size, sizeof(int), MSG_WAITALL);
			recv(socket, &malloc_request->memoria, size, MSG_WAITALL);

			return malloc_request;
		}
	}
	return NULL;
}
