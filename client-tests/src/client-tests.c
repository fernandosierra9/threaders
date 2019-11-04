#include "../../sac_cli/sac_cli.h"


// export LD_LIBRARY_PATH=/media/sf_tp-2019-2c-threaders/hilolay/Debug:/media/sf_tp-2019-2c-threaders/libmuse/Debug:/media/sf_tp-2019-2c-threaders/sac_cli/Debug:/media/sf_tp-2019-2c-threaders/shared_common/Debug
// ./client-tests -d -o direct_io --Disc-Path=/media/sf_tp-2019-2c-threaders/sac_server/disc.bin /media/sf_tp-2019-2c-threaders/client-tests/fuse_test
int main(int argc, char *argv[]) {
	/* HILOLAY CLIENTS */
	//lib_init();
	//th_create(test1);
	//th_create(test2);
	//th_return(0);

	/* MUSE CLIENTS */
	//muse_init((int)getpid(), "127.0.0.1", 5003);
    //uint32_t ptr = muse_alloc(200);
    //int dst;

    //muse_get(&dst, ptr, 4);
    //int src;
    //muse_cpy(ptr, &src, 12);
    //muse_free(ptr);
    //muse_close(); 

	/* SAC_CLI */
	sac_cli_init(argc, argv);
	//sac_cli_read("test");
	return 0;
}
