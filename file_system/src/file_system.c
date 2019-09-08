#include "file_system.h"

int main(void)
{
	int response = file_system_logger_create();
	if (response < 0)
		return response;

	response = file_system_config_load();
	if (response < 0)
	{
		file_system_logger_destroy();
		return response;
	}
	file_system_config_free();
	file_system_logger_destroy();
	return EXIT_SUCCESS;
}
