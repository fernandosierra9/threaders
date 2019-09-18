#include "memoria_logger.h"

t_log* memoria_log;

int memoria_logger_create()
{
	memoria_log = logger_create(LOG_FILE, PROGRAM_NAME);
	if (memoria_log == NULL || memoria_log < 0)
	{
		perror("No ha sido posible instanciar el memoria_logger");
		return -1;
	}

	logger_print_header(memoria_log, PROGRAM_NAME);

	return 0;
}

void memoria_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_info(memoria_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void memoria_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_warning(memoria_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void memoria_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_error(memoria_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void memoria_logger_destroy()
{
	logger_print_footer(memoria_log, PROGRAM_NAME);
	logger_destroy(memoria_log);
}

t_log* memoria_log_get()
{
	return memoria_log;
}
