#include "planificacion_logger.h"

t_log* planificacion_log;

int planificacion_logger_create()
{
	planificacion_log = logger_create(LOG_FILE, PROGRAM_NAME);
	if (planificacion_log == NULL || planificacion_log < 0)
	{
		perror("No ha sido posible instanciar el planificacion_logger");
		return -1;
	}

	logger_print_header(planificacion_log, PROGRAM_NAME);

	return 0;
}

void planificacion_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_info(planificacion_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void planificacion_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_warning(planificacion_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void planificacion_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	log_error(planificacion_log, string_from_vformat(message, arguments));
	va_end(arguments);
}

void planificacion_logger_destroy()
{
	logger_print_footer(planificacion_log, PROGRAM_NAME);
	logger_destroy(planificacion_log);
}

t_log* planificacion_log_get()
{
	return planificacion_log;
}
