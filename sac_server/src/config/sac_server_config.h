#ifndef CONFIG_SAC_SERVER_CONFIG_H_
#define CONFIG_SAC_SERVER_CONFIG_H_

#include <stdlib.h>
#include <commons/string.h>
#include "../../../shared-common/common/config.h"
#include "../logger/sac_server_logger.h"

#define CONFIG_FILE_PATH "sac_server.config"

typedef struct
{
	int listen_port;
} t_sac_config;

int sac_server_config_load();
void sac_server_config_free();
int sac_server_get_listen_port();

#endif /* CONFIG_SAC_SERVER_CONFIG_H_ */
