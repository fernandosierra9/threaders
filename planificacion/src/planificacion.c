#include "planificacion.h"

void planificacion_delay(int number_of_seconds);

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

	pthread_create(&main_thread, NULL, (void*) planificacion_init, NULL);
//	pthread_create(&scheduler_thread, NULL, (void*) planificacion_run_scheduler, NULL);

	pthread_cancel(main_thread);
//	pthread_cancel(scheduler_thread);

	planificacion_exit_gracefully();
	return EXIT_SUCCESS;
}

void planificacion_init()
{
//	socket_start_listening_select("127.0.0.1", planificacion_get_listen_port());
	while(1)
	{
		planificacion_delay(planificacion_get_metrics_timer());
		planificacion_logger_info("Printing metrics");
	}
	pthread_exit(0);
}

void planificacion_run_scheduler()
{
	while(1)
	{

	}
	pthread_exit(0);
}

void planificacion_delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

void planificacion_exit_gracefully()
{
	planificacion_config_free();
	planificacion_logger_destroy();
	exit(0);
}
