#ifndef CUSTOM_UTILITARIA_H_
#define CUSTOM_UTILITARIA_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <commons/string.h>

#define max(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define min(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })

#define ceiling(x,y) (((x) + (y) - 1) / (y))

void utils_end_string(char *string);
bool utils_is_empty(char* string);
char* utils_get_parameter_i(char** array, int i);
char* utils_get_extension(char* file_name);
char* utils_get_file_name(char* path);
bool utils_is_number(char* string);
char* utils_to_string_uint(uint16_t value);
char* utils_to_string_ul(unsigned value);
void utils_free_array(char** array);
char* utils_array_to_string(char** array);

#endif /* CUSTOM_UTILITARIA_H_ */
