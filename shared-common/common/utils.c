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
