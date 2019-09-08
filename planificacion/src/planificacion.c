#include "planificacion.h"

int main(void)
{
	int response = planificacion_logger_create();
	if (response < 0)
		return response;

	response = planificacion_config_load();
	if (response < 0)
	{
		planificacion_logger_destroy();
		return response;
	}
	planificacion_config_free();
	planificacion_logger_destroy();
	return EXIT_SUCCESS;
}
