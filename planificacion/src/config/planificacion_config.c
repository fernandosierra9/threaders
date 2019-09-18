#include "planificacion_config.h"

t_suse_config* planificacion_config_reload();
void read_config(t_config* config_file);
void print_config();

t_suse_config* planificacion_config;

int planificacion_config_load()
{
	planificacion_logger_info("Se establecerá la configuración");
	return config_load(planificacion_log_get(), CONFIG_FILE_PATH, read_config, print_config);
}

void planificacion_config_free()
{
	utils_free_array(planificacion_config->sem_ids);
	utils_free_array(planificacion_config->sem_init);
	utils_free_array(planificacion_config->sem_max);
	free(planificacion_config);
}

int planificacion_get_listen_port()
{
	return planificacion_config->listen_port;
}

int planificacion_get_metrics_timer()
{
	return planificacion_config->metrics_timer;
}

int planificacion_get_max_multiprog()
{
	return planificacion_config->max_multiprog;
}

char** planificacion_get_sem_ids()
{
	return planificacion_config->sem_ids;
}

char** planificacion_get_sem_init()
{
	return planificacion_config->sem_init;
}

char** planificacion_get_sem_max()
{
	return planificacion_config->sem_max;
}

double planificacion_get_alpha_sjf()
{
	return planificacion_config->alpha_sjf;
}

t_suse_config* planificacion_config_reload()
{
	planificacion_config_free();
	planificacion_config_load(planificacion_log_get(), CONFIG_FILE_PATH, read_config, print_config);
	return planificacion_config;
}

void read_config(t_config* config_file)
{
	planificacion_config = malloc(sizeof(t_suse_config));
	planificacion_config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
	planificacion_config->metrics_timer = config_get_int_value(config_file, "METRICS_TIMER");
	planificacion_config->max_multiprog = config_get_int_value(config_file, "MAX_MULTIPROG");
	planificacion_config->sem_ids = config_get_array_value(config_file, "SEM_IDS");
	planificacion_config->sem_init = config_get_array_value(config_file, "SEM_INIT");
	planificacion_config->sem_max = config_get_array_value(config_file, "SEM_MAX");
	planificacion_config->alpha_sjf = config_get_double_value(config_file, "ALPHA_SJF");
}

void print_config()
{
	planificacion_logger_info("LISTEN_PORT: %d", planificacion_config->listen_port);
	planificacion_logger_info("METRICS_TIMER: %d", planificacion_config->metrics_timer);
	planificacion_logger_info("MAX_MULTIPROG: %d", planificacion_config->max_multiprog);
	planificacion_logger_info("SEM_IDS: %s", utils_array_to_string(planificacion_config->sem_ids));
	planificacion_logger_info("SEM_INIT: %s", utils_array_to_string(planificacion_config->sem_init));
	planificacion_logger_info("SEM_MAX: %s", utils_array_to_string(planificacion_config->sem_max));
	planificacion_logger_info("ALPHA_SJF: %f", planificacion_config->alpha_sjf);
}
