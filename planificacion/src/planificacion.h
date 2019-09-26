#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include "common/sockets.h"
#include "config/planificacion_config.h"
#include "logger/planificacion_logger.h"

pthread_t main_thread;
pthread_t scheduler_thread;

void planificacion_init();
void planificacion_run_scheduler();
void planificacion_exit_gracefully();

#endif /* PLANIFICACION_H_ */
