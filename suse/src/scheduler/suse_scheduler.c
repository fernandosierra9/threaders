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
	pthread_mutex_init(&scheduler_mutex, NULL);
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
	pthread_mutex_destroy(&scheduler_mutex);
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
	pthread_mutex_lock(&scheduler_mutex);
	suse_logger_info("Grado actual de multiprogramación: %d", suse_get_max_multiprog());
	int new_size = list_size(queue_new);
	int ready_size = list_size(queue_ready);
	int run_size = thread_exec != NULL? 1: 0;
	int blocked_size = list_size(queue_blocked);
	pthread_mutex_unlock(&scheduler_mutex);
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

void scheduler_add_new_program(t_program* program)
{
	list_add(queue_new, program);
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

float scheduler_calculate_exponential_mean(int burst_time)
{
	float tn = 0.0f;
	float alpha = suse_get_alpha_sjf();
	//tn+1 = α*tn + (1 - α)*tn
	float next_tn = alpha * (float) burst_time + (1.0 - alpha) * tn;
	return next_tn;
}

void scheduler_run_long_term_scheduler()
{
	int i;
	for (i = 0; i < list_size(queue_new); i++)
	{
		t_thread* thread = list_get(queue_new, i);
		if (list_size(queue_ready) <= suse_get_max_multiprog())
		{
			list_remove(queue_new, i);
			list_add(queue_ready, thread);
			suse_decrease_multiprog();
		}
	}
}

t_thread* _scheduler_get_sjf(t_thread* th1, t_thread* th2) {
	float f1 = scheduler_calculate_exponential_mean(th1->cpu_time);
	float f2 = scheduler_calculate_exponential_mean(th2->cpu_time);
	return f1 <= f2 ? th1 : th2;
}

void scheduler_run_short_term_scheduler()
{
	int i;
	for (i = 0; i < list_size(queue_ready); i++)
	{
		t_thread* thread = list_get(queue_ready, i);
		t_thread* sjf_thread = (t_thread*) list_fold(queue_ready, thread, (void*) _scheduler_get_sjf);
		if (thread == sjf_thread)
		{
			list_remove(queue_ready, i);
			scheduler_execute_thread(sjf_thread);
			suse_increase_multiprog();
			return;
		}
	}
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
