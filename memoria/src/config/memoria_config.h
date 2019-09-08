#ifndef CONFIG_MEMORIA_CONFIG_H_
#define CONFIG_MEMORIA_CONFIG_H_

#include <stdlib.h>
#include <common/config.h>
#include <commons/config.h>
#include <commons/string.h>

#include "../logger/memoria_logger.h"

#define CONFIG_FILE_PATH "memoria.config"

typedef struct {
	int listen_port;
	int memory_size;
	int page_size;
	int swap_size;
} t_muse_config;

int memoria_config_load();
void memoria_config_free();
int memoria_get_listen_port();
int memoria_get_memory_size();
int memoria_get_page_size();
int memoria_get_swap_size();

#endif /* CONFIG_MEMORIA_CONFIG_H_ */
