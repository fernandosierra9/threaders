#ifndef LOGGER_MEMORIA_LOGGER_H_
#define LOGGER_MEMORIA_LOGGER_H_

#include <commons/log.h>
#include <common/logger.h>
#include <commons/string.h>
#include <stdarg.h>
#include <stdio.h>

#define LOG_FILE "memoria.log"
#define PROGRAM_NAME "MUSE"

int memoria_logger_create();
void memoria_logger_info(char* message, ...);
void memoria_logger_warn(char* message, ...);
void memoria_logger_error(char* message, ...);
void memoria_logger_destroy();
t_log* memoria_log_get();

#endif /* LOGGER_MEMORIA_LOGGER_H_ */
