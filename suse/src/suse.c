#include "suse.h"

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
	pthread_create(&scheduler_thread, NULL, (void*) suse_run_scheduler, NULL);

	pthread_join(scheduler_thread, NULL);

	suse_exit_gracefully();
	return EXIT_SUCCESS;
}

void suse_accept_incoming_program()
{
	int pid = scheduler_get_next_pid();
	t_program* program = program_create(pid);
	scheduler_add_new_program(program);
}

void suse_handle_command()
{

}

void suse_init()
{
	socket_start_listening_select(SUSE_IP, suse_get_listen_port());
	pthread_exit(0);
}

void suse_run_scheduler()
{
	scheduler_init();
	scheduler_init_semaphores();
	scheduler_execute_metrics();
	pthread_exit(0);
}

void suse_exit_gracefully()
{
	scheduler_destroy();
	suse_config_free();
	suse_logger_destroy();
	exit(0);
}
