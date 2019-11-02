#ifndef SCHEDULER_SUSE_SCHEDULER_H_
#define SCHEDULER_SUSE_SCHEDULER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include "../program/suse_program.h"
#include "../semaphore/suse_semaphore.h"
#include "../config/suse_config.h"
#include "../logger/suse_logger.h"

void scheduler_init();
void scheduler_init_semaphores();
void scheduler_add_new_program(t_program* program);
void scheduler_execute_metrics();
void scheduler_destroy();

pthread_mutex_t scheduler_mutex;

#endif /* SCHEDULER_SUSE_SCHEDULER_H_ */
