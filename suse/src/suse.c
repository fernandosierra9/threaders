#include "suse.h"

void suse_server_init();

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

void suse_init()
{
	//socket_start_listening_select(SUSE_IP, suse_get_listen_port());
	suse_server_init();
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
	int suse_socket;
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
		suse_logger_error("Error al establecer conexion con hilolay");
		return;
	}

	suse_logger_info("Conexion establecida con hilolay");
	int received_bytes;
	int protocol;
	while (1)
	{
		received_bytes = recv(hilolay_fd, &protocol, sizeof(int), 0);

		if (received_bytes < 0)
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

			case NEW_THREAD:
			{
				suse_logger_info("Recibi NEW_THREAD de hilolay");
				t_new_thread *newthread_recive =  utils_receive_and_deserialize(hilolay_fd,protocol);
				suse_logger_info("NT: Program ID: %d", newthread_recive->pid);
				suse_logger_info("NT: Thread ID: %d", newthread_recive->tid);

				t_program* program_id = program_create(newthread_recive->pid);
				t_thread* program_thread = program_create_thread(newthread_recive->pid, newthread_recive->tid);
				list_add(program_id->threads, program_thread);
				scheduler_add_new_program(program_id);
				break;
			}
			case THREAD_JOIN:
			{
				suse_logger_info("Recibi THREAD_JOIN de hilolay");
				t_thread_join *thread_join_recive =  utils_receive_and_deserialize(hilolay_fd,protocol);
				suse_logger_info("TJ: TID: %d", thread_join_recive->tid);
				break;
			}
			case THREAD_CLOSE:
			{
				suse_logger_info("Recibi THREAD_CLOSE de hilolay");
				t_thread_close *thread_close_recive =  utils_receive_and_deserialize(hilolay_fd,protocol);
				suse_logger_info("TC: TID: %d", thread_close_recive->tid);
				break;
			}
			case SEM_WAIT:
			{
				suse_logger_info("Recibi SEM_WAIT de hilolay");
				t_sem_wait *sem_wait_name = utils_receive_and_deserialize(hilolay_fd, protocol);
				suse_logger_info("SM: %s", sem_wait_name->semaphore);
				break;
			}
			case SEM_SIGNAL:
			{
				suse_logger_info("Recibi SEM_WAIT de hilolay");
				t_sem_signal *sem_signal_name = utils_receive_and_deserialize(hilolay_fd, protocol);
				suse_logger_info("SS: %s", sem_signal_name->semaphore);
				break;
			}

		}

	}
}


