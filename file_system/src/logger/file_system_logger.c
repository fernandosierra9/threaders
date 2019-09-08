#include "file_system_logger.h"

t_log* file_system_log;

int file_system_logger_create()
{
	file_system_log = logger_create(LOG_FILE, PROGRAM_NAME);
	if (file_system_log == NULL || file_system_log < 0)
	{
		perror("No ha sido posible instanciar el file_system_logger");
		return -1;
	}

	logger_print_header(file_system_log, PROGRAM_NAME);

	return 0;
}

void file_system_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_info(file_system_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void file_system_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_warning(file_system_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void file_system_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_error(file_system_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void file_system_logger_destroy()
{
	logger_print_footer(file_system_log, PROGRAM_NAME);
	logger_destroy(file_system_log);
}

t_log* file_system_log_get()
{
	return file_system_log;
}
