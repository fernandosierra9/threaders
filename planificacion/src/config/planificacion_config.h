#ifndef CONFIG_PLANIFICACION_CONFIG_H_
#define CONFIG_PLANIFICACION_CONFIG_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include "../../../shared-common/common/utils.h"
#include "../../../shared-common/common/config.h"
#include "../logger/planificacion_logger.h"

#define CONFIG_FILE_PATH "planificacion.config"

typedef struct {
	int listen_port;
	int metrics_timer;
	int max_multiprog;
	char** sem_ids;
	char** sem_init;
	char** sem_max;
	double alpha_sjf;
} t_suse_config;

int planificacion_config_load();
void planificacion_config_free();
int planificacion_get_listen_port();
int planificacion_get_metrics_timer();
int planificacion_get_max_multiprog();
char** planificacion_get_sem_ids();
char** planificacion_get_sem_init();
char** planificacion_get_sem_max();
double planificacion_get_alpha_sjf();

#endif /* CONFIG_PLANIFICACION_CONFIG_H_ */
