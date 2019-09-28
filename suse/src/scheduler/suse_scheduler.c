#include "suse_scheduler.h"

t_list* queue_new;
t_list* queue_ready;
t_list* queue_blocked;
t_list* queue_exit;

t_list* semphores;
t_list* programs;
t_list* threads;

t_thread* thread_exec;
t_program* program_exec;

int pid, tid = 0;

void scheduler_init()
{
	programs = list_create();
	threads = list_create();
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
	for(int i = 0; suse_get_sem_ids()[i] != NULL && suse_get_sem_init()[i] != NULL && suse_get_sem_max()[i] != NULL; i++)
	{
		char* sem_id = suse_get_sem_ids()[i];
		int sem_init = atoi(suse_get_sem_init()[i]);
		int sem_max = atoi(suse_get_sem_max()[i]);
		list_add(semphores, semaphore_create(sem_id, sem_init, sem_max));
		suse_logger_info("Se agrego un nuevo semaforo con ID: %s, init: %d y max: %d", sem_id, sem_init, sem_max);
	}
	suse_logger_info("Se han cargado los semaforos correctamente");
}

void scheduler_destroy()
{
	list_destroy_and_destroy_elements(queue_new, (void*)program_destroy);
	list_destroy_and_destroy_elements(queue_ready, (void*)program_destroy);
	list_destroy_and_destroy_elements(queue_blocked, (void*)program_destroy);
	list_destroy_and_destroy_elements(queue_exit, (void*)program_destroy);
	list_destroy_and_destroy_elements(semphores, (void*)semaphore_destroy);
	list_destroy_and_destroy_elements(programs, (void*)program_destroy);
	list_destroy_and_destroy_elements(threads, (void*)program_destroy_thread);
	suse_logger_info("Todas las estructuras de planificacion fueron destruidas!");
}

/**
 * METRICS
 * */

void _scheduler_list_threads(t_thread *t)
{
	suse_logger_info("Tiempo de ejecución: %d", t->exec_time);
	suse_logger_info("Tiempo de espera: %d", t->wait_time);
	suse_logger_info("Tiempo de uso de CPU: %d", t->cpu_time);
	suse_logger_info("Porcentaje del tiempo de ejecución: %d", t->exec_time_percent);
}

void _scheduler_list_programs(t_program *p)
{
	suse_logger_info("Grado actual de multiprogramación: %d", suse_get_max_multiprog());
	int new_size = list_size(queue_new);
	int ready_size = list_size(queue_ready);
	int run_size = list_size(queue_ready);
	int blocked_size = list_size(queue_blocked);
	suse_logger_info("Cantidad de hilos en NEW: %d, READY: %d, RUN: %d, BLOCKED: %d", new_size, ready_size, run_size, blocked_size);
}

void _scheduler_list_semaphores(t_semaphore *s)
{
	suse_logger_info("SemID: %s, valor actual: %d", s->id, s->value);
}

void scheduler_print_metrics()
{
	suse_logger_info("********METRICAS********");
	suse_logger_info("########POR HILOS########");
	list_iterate(threads, (void*) _scheduler_list_threads);
	suse_logger_info("########POR PROGRAMA########");
	list_iterate(programs, (void*) _scheduler_list_programs);
	suse_logger_info("########DEL SISTEMA########");
	list_iterate(semphores, (void*) _scheduler_list_semaphores);
	suse_logger_info("********FIN DE METRICAS********");
}

/**
 * PROGRAMS
 * */

t_program* _scheduler_find_program_by_id(int id) {
	int _is_the_program(t_program *p) {
		return id == p->pid;
	}

	return list_find(programs, (void*) _is_the_program);
}

void scheduler_execute_program(t_program* program)
{
	program->state = EXEC;
	program_exec = program;
}

void scheduler_finish_program(t_program* program)
{
	program->state = EXIT;
	list_add(queue_exit, program);
}

/**
 * THREADS
 * */

void scheduler_set_ready_thread(t_thread* thread)
{
	thread->state = READY;
	list_add(queue_ready, thread);
}

void scheduler_execute_thread(t_thread* thread)
{
	thread->state = EXEC;
	thread_exec = thread;
}

void scheduler_block_thread(t_thread* thread)
{
	thread->state = BLOCKED;
	list_add(queue_blocked, thread);
}

void scheduler_finish_thread(t_thread* thread)
{
	thread->state = EXIT;
	t_program* program = _scheduler_find_program_by_id(thread->pid);
	scheduler_finish_program(program);
	scheduler_print_metrics();
}

void scheduler_execute_metrics()
{
	while (1)
	{
		utils_delay(suse_get_metrics_timer());
		scheduler_print_metrics();
	}
}

int scheduler_get_next_pid()
{
	return pid++;
}

int scheduler_get_next_tid()
{
	return tid++;
}
