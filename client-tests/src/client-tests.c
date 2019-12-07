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

    printf("\n alloc: %d \n" , 200);
    uint32_t ptr = muse_alloc(200);
    printf("\n ptr nuevo: %d \n" , ptr);


    int num = 9;
    int des;
    muse_cpy(ptr, &num, sizeof(num));
    muse_get(&des, 5, 4);
    printf("Read: %d", des);




    char * test = strdup("fernando");
    char *result = malloc(strlen("fernando")+1);
    muse_cpy(ptr, test, strlen("fernando")+1);

    muse_get(result, ptr, strlen("fernando")+1);
    printf("Read: %s", result);



    //ptr = muse_alloc(200);

    printf("\n alloc: %d \n" , 1);
    ptr = muse_alloc(1);
    printf("\n ptr nuevo: %d \n" , ptr);

    printf("\n alloc: %d \n" , 1);
    ptr = muse_alloc(1);
    printf("\n ptr nuevo: %d \n" , ptr);

    ptr = muse_alloc(5);
    printf("\n ptr nuevo: %d \n" , ptr);

    ptr = muse_alloc(5);
    printf("\n ptr nuevo: %d \n" , ptr);


    printf("\n enviar map ");
    ptr =  muse_map("/home/utnso/tp-2019-2c-threaders/muse/Debug/test.txt", 774, MAP_SHARED);
    printf("\n ptr nuevo: %d \n" , ptr);



    /*
    char *  algo = malloc(strlen("fernando")+1);
    memcpy(algo,"fernando",strlen("fernando"));
    algo[strlen("fernando")] = '\0';
    muse_cpy(5, algo,strlen("fernando")+1);
	*/

    printf("enviar copy");

    muse_cpy(5, &num, sizeof(num));


    muse_get(&des, 5, 4);
    printf("Read: %d", des);

    muse_free(ptr);

    muse_close();
	//th_return(0);
	return 0;
}
