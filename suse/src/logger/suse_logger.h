#ifndef LOGGER_SUSE_LOGGER_H_
#define LOGGER_SUSE_LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../../shared-common/common/logger.h"

#define LOG_FILE "suse.log"
#define PROGRAM_NAME "SUSE"

int suse_logger_create();
void suse_logger_info(char* message, ...);
void suse_logger_warn(char* message, ...);
void suse_logger_error(char* message, ...);
void suse_logger_destroy();
t_log* suse_log_get();

#endif /* LOGGER_SUSE_LOGGER_H_ */
