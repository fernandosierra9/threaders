#include "sac_server_config.h"

t_fuse_config* sac_server_config_reload();
void read_config(t_config* config_file);
void print_config();

t_fuse_config* sac_server_config;

int sac_server_config_load()
{
	sac_server_logger_info("Se establecerá la configuración");
	return config_load(sac_server_log_get(), CONFIG_FILE_PATH, read_config, print_config);
}

void sac_server_config_free()
{
	free(sac_server_config);
}

int sac_server_get_listen_port()
{
	return sac_server_config->listen_port;
}

t_fuse_config* sac_server_config_reload()
{
	sac_server_config_free();
	sac_server_config_load(sac_server_log_get(), CONFIG_FILE_PATH, read_config, print_config);
	return sac_server_config;
}

void read_config(t_config* config_file)
{
	sac_server_config = malloc(sizeof(t_fuse_config));
	sac_server_config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
}

void print_config()
{
	sac_server_logger_info("LISTEN_PORT: %d", sac_server_config->listen_port);
}
