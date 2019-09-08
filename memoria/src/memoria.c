#include "memoria.h"

int main(void) {
	int response = memoria_logger_create();
	if (response < 0)
		return response;

	response = memoria_config_load();
	if (response < 0)
	{
		memoria_logger_destroy();
		return response;
	}
	memoria_config_free();
	memoria_logger_destroy();
	return EXIT_SUCCESS;
}
