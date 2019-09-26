#include "suse_logger.h"

t_log* suse_log;

int suse_logger_create()
{
	suse_log = logger_create(LOG_FILE, PROGRAM_NAME);
	if (suse_log == NULL || suse_log < 0)
	{
		perror("No ha sido posible instanciar el suse_logger");
		return -1;
	}

	logger_print_header(suse_log, PROGRAM_NAME);

	return 0;
}

void suse_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_info(suse_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void suse_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_warning(suse_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void suse_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_error(suse_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void suse_logger_destroy()
{
	logger_print_footer(suse_log, PROGRAM_NAME);
	logger_destroy(suse_log);
}

t_log* suse_log_get()
{
	return suse_log;
}
