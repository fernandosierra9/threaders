#include "utils.h"

void utils_end_string(char *string)
{
	if ((strlen(string) > 0) && (string[strlen(string) - 1] == '\n'))
		string[strlen(string) - 1] = '\0';
}

bool utils_is_empty(char* string)
{
	if (string == NULL || string_is_empty(string))
		return true;
	return false;
}

char* utils_get_parameter_i(char** array, int i)
{
	if (array[i] != NULL)
		return array[i];
	return "";
}

char* utils_get_extension(char* file_name)
{
	char *extension = strrchr(file_name, '.');
	if (!extension || extension == file_name)
		return "";
	return extension + 1;
}

char* utils_get_file_name(char* path)
{
	char *file = strrchr(path, '/');
	if (!file || file == path)
		return "";
	return file + 1;
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

char* utils_to_string_uint(uint16_t value)
{
	char* aux = malloc(sizeof(char));
	sprintf(aux,"%u",value);
	return aux;
}

char* utils_to_string_ul(unsigned value)
{
	char* aux = malloc(sizeof(char));
	sprintf(aux,"%u",value);
	return aux;
}

void utils_free_array(char** array)
{
	unsigned int i = 0;
	for(; array[i] != NULL; i++)
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
	string_append(&ret, "[");
	while(array[i] != NULL)
	{
		aux = array[i];
		string_append(&ret, aux);
		free(aux);
		if (array[i + 1] != NULL)
		{
			string_append(&ret, ",");
		}
		i++;
	}
	string_append(&ret, "]");
	return ret;
}

void utils_buffer_create(t_package* package)
{
	package->buffer = malloc(sizeof(t_buffer));
	package->buffer->size = 0;
	package->buffer->stream = NULL;
}

t_package* utils_package_create(void)
{
	t_package* package = malloc(sizeof(t_package));
	package->operation_code = PACKAGE;
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

void utils_send_message(char* message, int client_socket)
{
	t_package* package = malloc(sizeof(t_package));

	package->operation_code = MESSAGE;
	package->buffer = malloc(sizeof(t_buffer));
	package->buffer->size = strlen(message) + 1;
	package->buffer->stream = malloc(package->buffer->size);
	memcpy(package->buffer->stream, message, package->buffer->size);

	int bytes = package->buffer->size + 2*sizeof(int);

	void* to_send = serializer_serialize_package(package, bytes);

	send(client_socket, to_send, bytes, 0);

	free(to_send);
	utils_package_destroy(package);
}

void utils_package_send_to(t_package* t_package, int client_socket)
{
	int bytes = t_package->buffer->size + 2*sizeof(int);
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
		t_package* package = malloc(sizeof(t_package));
		package->operation_code = MALLOC;
		package->buffer = malloc(sizeof(t_buffer));
		package->buffer->size = sizeof(uint32_t);
		package->buffer->stream = malloc(package->buffer->size);
		memcpy(package->buffer->stream, &((t_malloc*) package)->memoria, package->buffer->size);
		int bytes = package->buffer->size + 2 * sizeof(int);
		void* to_send = serializer_serialize_package(package, bytes);
		send(socket, to_send, bytes, 0);
		free(to_send);
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
