#ifndef LOGGER_FILE_SYSTEM_LOGGER_H_
#define LOGGER_FILE_SYSTEM_LOGGER_H_

#include <commons/log.h>
#include "../../../shared-common/common/logger.h"
#include <commons/string.h>
#include <stdarg.h>
#include <stdio.h>

#define LOG_FILE "file_system.log"
#define PROGRAM_NAME "FUSE"

int file_system_logger_create();
void file_system_logger_info(char* message, ...);
void file_system_logger_warn(char* message, ...);
void file_system_logger_error(char* message, ...);
void file_system_logger_destroy();
t_log* file_system_log_get();

#endif /* LOGGER_FILE_SYSTEM_LOGGER_H_ */
