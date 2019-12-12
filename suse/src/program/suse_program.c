#include "suse_program.h"

t_program* program_create(int pid)
{
	t_program* program = malloc(sizeof(t_program));
	program->pid = pid;
	program->threads = list_create();
	program->thread_exec = NULL;
	program->queue_ready = list_create();
	return program;
}

void program_destroy(t_program* program)
{
	list_destroy_and_destroy_elements(program->threads, (void*)program_destroy_thread);
	list_clean(program->queue_ready);
	free(program);
}

t_thread* program_create_thread(int pid, int tid)
{
	t_thread* thread = malloc(sizeof(t_thread));
	thread->pid = pid;
	thread->tid = tid;
	thread->cpu_time = 0;
	thread->exec_time = 0;
	thread->exec_time_percent = 0;
	thread->wait_time = 0;
	thread->state = NEW;
	return thread;
}

void program_destroy_thread(t_thread* thread)
{
	free(thread);
}
