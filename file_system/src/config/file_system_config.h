#ifndef CONFIG_FILE_SYSTEM_CONFIG_H_
#define CONFIG_FILE_SYSTEM_CONFIG_H_

#include <stdlib.h>
#include "../../../shared-common/common/logger.h"
#include <commons/config.h>
#include <commons/string.h>

#include "../logger/file_system_logger.h"

#define CONFIG_FILE_PATH "file_system.config"

typedef struct
{
	int listen_port;
} t_fuse_config;

int file_system_config_load();
void file_system_config_free();
int file_system_get_listen_port();

#endif /* CONFIG_FILE_SYSTEM_CONFIG_H_ */
