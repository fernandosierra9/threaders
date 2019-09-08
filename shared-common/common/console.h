#ifndef COMMON_CONSOLE_H_
#define COMMON_CONSOLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>

#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/string.h>

#define QUIT_KEY "QUIT"
#define EXIT_KEY "EXIT"
#define SPLIT_CHAR " "
#define INPUT_SIZE 50

typedef struct {
	char *key;
	void (*action)(char**, int);
	char* (*get_table_name)(t_list* arguments);
} t_command;

int console_read(t_log* log, t_dictionary*);

#endif /* COMMON_CONSOLE_H_ */
