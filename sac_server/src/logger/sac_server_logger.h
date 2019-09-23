#ifndef LOGGER_SAC_SERVER_LOGGER_H_
#define LOGGER_SAC_SERVER_LOGGER_H_

#include <commons/log.h>
#include "../../../shared-common/common/logger.h"
#include <commons/string.h>
#include <stdarg.h>
#include <stdio.h>

#define LOG_FILE "sac_server.log"
#define PROGRAM_NAME "sac-server"

int sac_server_logger_create();
void sac_server_logger_info(char* message, ...);
void sac_server_logger_warn(char* message, ...);
void sac_server_logger_error(char* message, ...);
void sac_server_logger_destroy();
t_log* sac_server_log_get();

#endif /* LOGGER_SAC_SERVER_LOGGER_H_ */
