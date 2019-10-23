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
	//socket_start_listening_select(SUSE_IP, suse_get_listen_port());
	suse_server_init(SUSE_IP, suse_get_listen_port());
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


void suse_server_init()
{
	suse_socket = socket_create_listener("127.0.0.1", suse_get_listen_port());
	if (suse_socket < 0)
	{
		suse_logger_error("Error al crear server");
		return;
	}

	suse_logger_info("Esperando conexion de hilolay");

	int hilolay_fd = socket_accept_conection(suse_socket);

	if (hilolay_fd == -1)
	{
		suse_logger_error("Error al establecer conexion con el libmuse");
		return;
	}


	suse_logger_info("Conexion establecida con libmuse");
	int received_bytes;
	int protocol;
	while (1)
	{
		received_bytes = recv(hilolay_fd, &protocol, sizeof(int), 0);

		if (received_bytes <= 0)
		{
			suse_logger_error("Error al recibir la operacion de hilolay");
			suse_logger_error("Se desconecto hilolay");
			exit(1);
		}
		switch (protocol)
		{
			case HANDSHAKE:
			{
				suse_logger_info("Recibi malloc de hilolay");
				break;
			}

		}

	}
}


