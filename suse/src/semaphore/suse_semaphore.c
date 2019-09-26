#include "suse_semaphore.h"

t_semaphore* semaphore_create(char* id, int value)
{
	t_semaphore* sem = malloc(sizeof(t_semaphore));
	strncpy(sem->id, id, sizeof(id));
	sem->value = value;
	return sem;
}
