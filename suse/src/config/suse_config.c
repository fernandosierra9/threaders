#include "suse_config.h"

t_suse_config* suse_config_reload();
void read_config(t_config* config_file);
void print_config();

t_suse_config* suse_config;

char** sem_ids;
char** sem_init;
char** sem_max;

int suse_config_load()
{
	suse_logger_info("Se establecerá la configuración");
	return config_load(suse_log_get(), CONFIG_FILE_PATH, read_config, print_config);
}

void suse_config_free()
{
	utils_free_array(sem_ids);
	utils_free_array(sem_init);
	utils_free_array(sem_max);
	free(suse_config);
}

int suse_get_listen_port()
{
	return suse_config->listen_port;
}

int suse_get_metrics_timer()
{
	return suse_config->metrics_timer;
}

int suse_get_max_multiprog()
{
	return suse_config->max_multiprog;
}

char** suse_get_sem_ids()
{
	return sem_ids;
}

char** suse_get_sem_init()
{
	return sem_init;
}

char** suse_get_sem_max()
{
	return sem_max;
}

double suse_get_alpha_sjf()
{
	return suse_config->alpha_sjf;
}

void suse_decrease_multiprog()
{
	suse_config->max_multiprog--;
}

void suse_increase_multiprog()
{
	suse_config->max_multiprog++;
}

t_suse_config* suse_config_reload()
{
	suse_config_free();
	suse_config_load(suse_log_get(), CONFIG_FILE_PATH, read_config, print_config);
	return suse_config;
}

void read_config(t_config* config_file)
{
	suse_config = malloc(sizeof(t_suse_config));
	suse_config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
	suse_config->metrics_timer = config_get_int_value(config_file, "METRICS_TIMER");
	suse_config->max_multiprog = config_get_int_value(config_file, "MAX_MULTIPROG");
	suse_config->sem_ids = config_get_array_value(config_file, "SEM_IDS");
	sem_ids = config_get_array_value(config_file, "SEM_IDS");
	suse_config->sem_init = config_get_array_value(config_file, "SEM_INIT");
	sem_init = config_get_array_value(config_file, "SEM_INIT");;
	suse_config->sem_max = config_get_array_value(config_file, "SEM_MAX");
	sem_max =  config_get_array_value(config_file, "SEM_MAX");
	suse_config->alpha_sjf = config_get_double_value(config_file, "ALPHA_SJF");
}

void print_config()
{
	suse_logger_info("LISTEN_PORT: %d", suse_config->listen_port);
	suse_logger_info("METRICS_TIMER: %d", suse_config->metrics_timer);
	suse_logger_info("MAX_MULTIPROG: %d", suse_config->max_multiprog);
	char* sem_ids =  utils_array_to_string(suse_config->sem_ids);
	suse_logger_info("SEM_IDS: %s", sem_ids);
	free(sem_ids);
	char* sem_init = utils_array_to_string(suse_config->sem_init);
	suse_logger_info("SEM_INIT: %s", sem_init);
	free(sem_init);
	char* sem_max =  utils_array_to_string(suse_config->sem_max);
	suse_logger_info("SEM_MAX: %s", sem_max);
	free(sem_max);
	suse_logger_info("ALPHA_SJF: %f", suse_config->alpha_sjf);
}
