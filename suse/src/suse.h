#ifndef SUSE_H_
#define SUSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <assert.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include "common/sockets.h"
#include "config/suse_config.h"
#include "logger/suse_logger.h"
#include "scheduler/suse_scheduler.h"

#define SUSE_IP "127.0.0.1"

pthread_t main_thread;
pthread_t scheduler_thread;

void suse_init();
void suse_run_scheduler();
void suse_exit_gracefully();

#endif /* SUSE_H_ */
