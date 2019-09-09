#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <assert.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include "memoria.h"
#include "config.h"
#include "logger.h"

//#include "../shared-common/conector.h"

/* -- Global variables -- */
t_log *logger;
struct setup_t setup;

/* Local functions */
static void init_server(int port);

int main(void) {
	init_config();
	exit_gracefully(EXIT_SUCCESS);
}

void exit_gracefully(int status) {
	exit(status);
}

