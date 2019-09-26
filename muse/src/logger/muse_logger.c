#include "muse_logger.h"

int muse_logger_create()
{
	muse_log = logger_create(LOG_FILE, PROGRAM_NAME);
	if (muse_log == NULL || muse_log < 0)
	{
		perror("No ha sido posible instanciar el muse_logger");
		return -1;
	}

	logger_print_header(muse_log, PROGRAM_NAME);

	return 0;
}

void muse_logger_info(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_info(muse_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void muse_logger_warn(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_warning(muse_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void muse_logger_error(char* message, ...)
{
	va_list arguments;
	va_start(arguments, message);
	char* formated_message = string_from_vformat(message, arguments);
	log_error(muse_log, formated_message);
	free(formated_message);
	va_end(arguments);
}

void muse_logger_destroy()
{
	logger_print_footer(muse_log, PROGRAM_NAME);
	logger_destroy(muse_log);
}

t_log* muse_log_get()
{
	return muse_log;
}
