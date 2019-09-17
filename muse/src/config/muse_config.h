#ifndef CONFIG_MUSE_CONFIG_H_
#define CONFIG_MUSE_CONFIG_H_

#include <stdlib.h>
#include <common/config.h>
#include <commons/config.h>
#include <commons/string.h>

#include "../logger/muse_logger.h"

#define CONFIG_FILE_PATH "muse.config"

typedef struct
{	int listen_port;
	int memory_size;
	int page_size;
	int swap_size;

} t_muse_config;

t_muse_config* muse_config;

int muse_config_load();
void muse_config_free();
int muse_get_listen_port();

#endif /* CONFIG_MUSE_CONFIG_H_ */
