#ifndef LOGGER_PLANIFICACION_LOGGER_H_
#define LOGGER_PLANIFICACION_LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../../shared-common/common/logger.h"

#define LOG_FILE "planificacion.log"
#define PROGRAM_NAME "SUSE"

int planificacion_logger_create();
void planificacion_logger_info(char* message, ...);
void planificacion_logger_warn(char* message, ...);
void planificacion_logger_error(char* message, ...);
void planificacion_logger_destroy();
t_log* planificacion_log_get();

#endif /* LOGGER_PLANIFICACION_LOGGER_H_ */
