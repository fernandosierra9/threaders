#ifndef CONFIG_SUSE_CONFIG_H_
#define CONFIG_SUSE_CONFIG_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include "../../../shared-common/common/utils.h"
#include "../../../shared-common/common/config.h"
#include "../logger/suse_logger.h"

#define CONFIG_FILE_PATH "suse.config"

typedef struct {
	int listen_port;
	int metrics_timer;
	int max_multiprog;
	char** sem_ids;
	char** sem_init;
	char** sem_max;
	double alpha_sjf;
} t_suse_config;

int suse_config_load();
void suse_config_free();
int suse_get_listen_port();
int suse_get_metrics_timer();
int suse_get_max_multiprog();
char** suse_get_sem_ids();
char** suse_get_sem_init();
char** suse_get_sem_max();
double suse_get_alpha_sjf();
void suse_decrease_multiprog();
void suse_increase_multiprog();

#endif /* CONFIG_SUSE_CONFIG_H_ */
