#include "suse.h"

void suse_delay(int number_of_seconds);

int main(void)
{
	int response = suse_logger_create();
	if (response < 0)
		return response;

	response = suse_config_load();
	if (response < 0)
	{
		suse_logger_destroy();
		return response;
	}

	pthread_create(&main_thread, NULL, (void*) suse_init, NULL);
//	pthread_create(&scheduler_thread, NULL, (void*) suse_run_scheduler, NULL);

	pthread_cancel(main_thread);
//	pthread_cancel(scheduler_thread);

	suse_exit_gracefully();
	return EXIT_SUCCESS;
}

void suse_init()
{
//	socket_start_listening_select("127.0.0.1", suse_get_listen_port());
	while(1)
	{
		suse_delay(suse_get_metrics_timer());
		suse_logger_info("Printing metrics");
	}
	pthread_exit(0);
}

void suse_run_scheduler()
{
	while(1)
	{

	}
	pthread_exit(0);
}

void suse_delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

void suse_exit_gracefully()
{
	suse_config_free();
	suse_logger_destroy();
	exit(0);
}
