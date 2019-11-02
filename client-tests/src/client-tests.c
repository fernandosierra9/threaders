#include "../../hilolay/hilolay.h"
#include "../../libmuse/libmuse.h"

void recursiva(int cant)
{
	if (cant > 0)
		recursiva(cant - 1);
}

void test1()
{
	int i, tid;

	for (i = 0; i < 10; i++)
	{
		tid = th_get_tid();
		printf("Soy el ult %d mostrando el numero %d \n", tid, i);
		/* Randomizes the sleep, so it gets larger after a few iterations */
		usleep(5000 * i * tid);

		recursiva(i);

		// Round Robin will yield the CPU
		if ((i + tid) % 5 == 0)
			th_yield();
	}
}

void test2()
{
	int i, tid;

	for (i = 0; i < 5; i++)
	{
		tid = th_get_tid();
		printf("Soy el ult %d mostrando el numero %d \n", tid, i);
		/* Randomizes the sleep, so it gets larger after a few iterations */
		usleep(2000 * i * tid);
		recursiva(i);
	}
}

int main(void)
{
	/* HILOLAY CLIENTS */
	//lib_init();
	//th_create(test1);
	//th_create(test2);

	/* MUSE CLIENTS */
    muse_init((int)getpid(),
    		"127.0.0.1", 5003);
    uint32_t ptr = muse_alloc(200);
    printf("\n ptr nuevo: %d \n" , ptr);
    ptr = muse_alloc(200);
    printf("\n ptr nuevo: %d \n" , ptr);
    ptr = muse_alloc(100);
    printf("\n ptr nuevo: %d \n" , ptr);
    ptr = muse_alloc(50);
    printf("\n ptr nuevo: %d \n" , ptr);

    int num = 4;
    muse_cpy(ptr, &num, sizeof(num));
    int des;

    muse_get(&des, ptr, 4);
    printf("Read: %d", des);

    muse_free(ptr);

    muse_close();
	//th_return(0);
	return 0;
}
