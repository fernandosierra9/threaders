#include "sac_server_logger.h"

t_log* sac_server_log;

int sac_server_logger_create()
{
	sac_server_log = logger_create(LOG_FILE, PROGRAM_NAME);
	if (sac_server_log == NULL || sac_server_log < 0)
	{
		perror("No ha sido posible instanciar el sac_server_logger");
		return -1;
	}

	logger_print_header(sac_server_log, PROGRAM_NAME);

	return 0;
}

void sac_server_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_info(sac_server_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void sac_server_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_warning(sac_server_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void sac_server_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_error(sac_server_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void sac_server_logger_destroy()
{
	logger_print_footer(sac_server_log, PROGRAM_NAME);
	logger_destroy(sac_server_log);
}

t_log* sac_server_log_get()
{
	return sac_server_log;
}
