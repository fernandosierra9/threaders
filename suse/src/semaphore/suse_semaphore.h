#ifndef SEMAPHORE_SUSE_SEMAPHORE_H_
#define SEMAPHORE_SUSE_SEMAPHORE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/queue.h>
#include "../program/suse_program.h"

typedef struct
{
	char* id;
	int value;
	int max_value;
	t_queue* queue;
} t_semaphore;

t_semaphore* semaphore_create(char* id, int value, int max_value);
void semaphore_lock_thread(t_semaphore* sem, t_thread* thread);
t_thread* semaphore_unlock_thread(t_semaphore* sem);
void semaphore_destroy(t_semaphore* sem);

#endif /* SEMAPHORE_SUSE_SEMAPHORE_H_ */
