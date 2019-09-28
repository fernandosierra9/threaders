#include "suse_semaphore.h"

t_semaphore* semaphore_create(char* id, int value, int max_value)
{
	t_semaphore* sem = malloc(sizeof(t_semaphore));
	sem->id = malloc(sizeof(id));
	strcpy(sem->id, id);
	sem->value = value;
	sem->max_value = max_value;
	sem->queue = queue_create();
	return sem;
}

void semaphore_destroy(t_semaphore* sem)
{
	free(sem->id);
	queue_destroy(sem->queue);
	free(sem);
}
