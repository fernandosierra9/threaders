#include "suse_semaphore.h"

t_semaphore* semaphore_create(char* id, int value, int max_value)
{
	t_semaphore* sem = malloc(sizeof(t_semaphore));
	strncpy(sem->id, id, sizeof(id));
	sem->value = value;
	sem->max_value = max_value;
	return sem;
}

void semaphore_destroy(t_semaphore* sem)
{
	free(sem->id);
	free(sem);
}
