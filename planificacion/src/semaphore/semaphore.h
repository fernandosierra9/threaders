#ifndef SEMAPHORE_SEMAPHORE_H_
#define SEMAPHORE_SEMAPHORE_H_

typedef struct
{
	char* id;
	int value;
}t_semaphore;

t_semaphore* semaphore_create(char* id, int value);

#endif /* SEMAPHORE_SEMAPHORE_H_ */
