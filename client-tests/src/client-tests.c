#include "../../hilolay/hilolay.h"
#include "../../libmuse/libmuse.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void recursiva(int num)
{
	if(num == 0)
		return;

	uint32_t ptr = muse_alloc(4);
	muse_cpy(ptr, &num, 4);
	printf("%d copy\n", num);

	recursiva(num - 1);
	int algo = 0; // Se pisa para probar que muse_get cargue el valor adecuado
	printf("%d ptr\n", ptr);
	muse_get(&algo, ptr, 4);
	printf("%d algo\n", algo);

}

int main(void)
{
	muse_init(getpid(), "127.0.0.1", 5003);
	recursiva(10);
	muse_close();
}
