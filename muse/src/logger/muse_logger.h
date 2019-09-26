#ifndef LOGGER_MUSE_LOGGER_H_
#define LOGGER_MUSE_LOGGER_H_


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../../shared-common/common/logger.h"

#define LOG_FILE "muse.log"
#define PROGRAM_NAME "MUSE"

int  muse_logger_create();
void muse_logger_info(char* message, ...);
void muse_logger_warn(char* message, ...);
void muse_logger_error(char* message, ...);
void muse_logger_destroy();
t_log* muse_log_get();

t_log* muse_log;
#endif /* LOGGER_MUSE_LOGGER_H_ */
