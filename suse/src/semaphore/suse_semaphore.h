#ifndef SEMAPHORE_SUSE_SEMAPHORE_H_
#define SEMAPHORE_SUSE_SEMAPHORE_H_

typedef struct
{
	char* id;
	int value;
	int max_value;
} t_semaphore;

t_semaphore* semaphore_create(char* id, int value, int max_value);
void semaphore_destroy(t_semaphore* sem);

#endif /* SEMAPHORE_SUSE_SEMAPHORE_H_ */
