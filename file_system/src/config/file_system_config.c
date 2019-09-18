#include "file_system_config.h"

t_fuse_config* file_system_config_reload();
void read_config(t_config* config_file);
void print_config();

t_fuse_config* file_system_config;

int file_system_config_load()
{
	file_system_logger_info("Se establecerá la configuración");
	return config_load(file_system_log_get(), CONFIG_FILE_PATH, read_config, print_config);
}

void file_system_config_free()
{
	free(file_system_config);
}

int file_system_get_listen_port()
{
	return file_system_config->listen_port;
}

t_fuse_config* file_system_config_reload()
{
	file_system_config_free();
	file_system_config_load(file_system_log_get(), CONFIG_FILE_PATH, read_config, print_config);
	return file_system_config;
}

void read_config(t_config* config_file)
{
	file_system_config = malloc(sizeof(t_fuse_config));
	file_system_config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
}

void print_config()
{
	file_system_logger_info("LISTEN_PORT: %d", file_system_config->listen_port);
}
