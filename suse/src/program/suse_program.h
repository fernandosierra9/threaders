#ifndef SUSE_PROGRAM_H_
#define SUSE_PROGRAM_H_

#include <stdio.h>
#include <stdlib.h>
#include "commons/collections/list.h"

typedef enum
{
	NEW, READY, EXEC, BLOCKED, EXIT
} e_states;

typedef struct
{
	int pid;
	int tid;
	e_states state;
	int exec_time;
	int wait_time;
	int cpu_time;
	int exec_time_percent;
} t_thread;

typedef struct
{
	int pid;
	e_states state;
	t_list* threads;
} t_program;

t_program* program_create(int pid);
void program_destroy(t_program* program);
t_thread* program_create_thread(int pid, int tid);
void program_destroy_thread(t_thread* thread);

#endif /* SUSE_PROGRAM_H_ */
