#include "console.h"

char* get_input();
char** get_arguments_from_input(char*, int);
int get_arguments_size(char *input);
void command_execute(t_log* log, char *key, t_dictionary* command_actions, char** arguments, int arguments_size);

int console_read(t_log* log, t_dictionary* command_actions)
{
	char* input = get_input();
	if (input == NULL)
		return 0;

	string_to_upper(input);
	int arguments_size = get_arguments_size(input);

	char** arguments = get_arguments_from_input(input, arguments_size);

	char* key = string_duplicate(arguments[0]);

	if (string_equals_ignore_case(key, EXIT_KEY)
			|| string_equals_ignore_case(key, QUIT_KEY))
		return -1;
	else
		command_execute(log, key, command_actions, arguments, arguments_size);

	free(arguments);
	free(input);

	return 0;
}

char* get_input()
{
	char *input = malloc(INPUT_SIZE);
	fgets(input, INPUT_SIZE, stdin);

	string_trim(&input);

	if (string_is_empty(input)) {
		free(input);
		return NULL;
	}

	return input;
}

int get_arguments_size(char *input)
{
	char last_char = 'a';
	int count = 1;
	for (int i = 0; i < string_length(input); i++)
	{
		if (i != 0)
			last_char = input[i - 1];
		if (input[i] == ' ' && last_char != ' ')
			count++;
	}

	return count;
}

char** get_arguments_from_input(char* input, int arguments_size) {
	char** arguments = malloc(sizeof(char*) * arguments_size);

	if (string_contains(input, SPLIT_CHAR))
		arguments = string_split(input, SPLIT_CHAR);
	else
		arguments[0] = string_duplicate(input);

	return arguments;
}

void command_execute(t_log* log, char *key, t_dictionary* command_actions,
		char** arguments, int arguments_size) {
	t_command* command = dictionary_get(command_actions, key);
	if (command == NULL)
		log_warning(log, "Comando %s desconocido", key);
	else
		command->action(arguments, arguments_size);
}
