#include "suse_scheduler.h"

t_list* queue_new;
t_list* queue_ready;
t_list* queue_blocked;
t_list* queue_exit;

t_list* semphores;

t_thread* thread_exec;

int pid, tid = 0;

void scheduler_init()
{
	queue_new = list_create();
	queue_ready = list_create();
	queue_blocked = list_create();
	queue_exit = list_create();
	suse_logger_info("Todas las colas de estados fueron creadas!");
}

void scheduler_init_semaphores()
{
	semphores = list_create();
	suse_logger_info("Se creo lista de semaforos..");
	int i = 0;
	while (suse_get_sem_ids()[i] != NULL)
	{
		while (suse_get_sem_init()[i] != NULL)
		{
			while (suse_get_sem_max()[i] != NULL)
			{
				char* sem_id = suse_get_sem_ids()[i];
				int sem_init = atoi(suse_get_sem_init()[i]);
				int sem_max = atoi(suse_get_sem_max()[i]);
				list_add(semphores, semaphore_create(sem_id, sem_init, sem_max));
				suse_logger_info("Se agrego un nuevo semaforo con ID: %s, init: %d y max: %d", sem_id, sem_init, sem_max);
				free(sem_id);
			}
		}
		i++;
	}
}

void scheduler_destroy()
{
	list_destroy_and_destroy_elements(queue_new, (void*)program_destroy);
	list_destroy_and_destroy_elements(queue_ready, (void*)program_destroy);
	list_destroy_and_destroy_elements(queue_blocked, (void*)program_destroy);
	list_destroy_and_destroy_elements(queue_exit, (void*)program_destroy);
	list_destroy_and_destroy_elements(semphores, (void*)semaphore_destroy);
	suse_logger_info("Todas las estructuras de planificacion fueron destruidas!");
}

float calcularMediaExponencial(int duracionRafaga, float estimacionTn) {
	float estimacionTnMasUno = 0.0f;
	estimacionTnMasUno = suse_get_alpha_sjf() * (float)duracionRafaga + (1.0 - suse_get_alpha_sjf()) * estimacionTn;

	return estimacionTnMasUno;
}

void scheduler_execute_metrics()
{
	//Por hilo
	suse_logger_info("Tiempo de ejecución: %d", 10);
	suse_logger_info("Tiempo de espera: %d", 10);
	suse_logger_info("Tiempo de uso de CPU: %d", 10);
	suse_logger_info("Porcentaje del tiempo de ejecución: %d", 10);
	//Por programa
	suse_logger_info("Grado actual de multiprogramación: %d", 10);
	suse_logger_info("Cantidad de hilos en NEW: %d, READY: %d, RUN: %d, BLOCKED: %d", 10, 10, 10, 10);
	//Sistema
	//Por semaforo
	suse_logger_info("SemID: %s, valor actual: %d", "A", 10);
}

int scheduler_get_next_pid()
{
	return pid++;
}

int scheduler_get_next_tid()
{
	return tid++;
}
