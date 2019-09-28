#include "suse_program.h"

t_program* program_create(int pid)
{
	t_program* program = malloc(sizeof(t_program));
	program->pid = pid;
	program->state = NEW;
	program->threads = malloc(sizeof(t_thread));
	return program;
}

void program_destroy(t_program* program)
{
	free(program->threads);
	free(program);
}

t_thread* program_create_thread(int pid, int tid)
{
	t_thread* thread = malloc(sizeof(t_thread));
	thread->pid = pid;
	thread->tid = tid;
	thread->state = READY;
	return thread;
}

void program_destroy_thread(t_thread* thread)
{
	free(thread);
}
