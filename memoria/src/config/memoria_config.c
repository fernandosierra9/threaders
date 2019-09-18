#include "memoria_config.h"

t_muse_config* memoria_config_reload();
void read_config(t_config* config_file);
void print_config();

t_muse_config* memoria_config;

int memoria_config_load()
{
	memoria_logger_info("Se establecerá la configuración");
	return config_load(memoria_log_get(), CONFIG_FILE_PATH, read_config, print_config);
}

void memoria_config_free()
{
	free(memoria_config);
}

int memoria_get_listen_port()
{
	return memoria_config->listen_port;
}

int memoria_get_memory_size()
{
	return memoria_config->memory_size;
}

int memoria_get_page_size()
{
	return memoria_config->page_size;
}

int memoria_get_swap_size()
{
	return memoria_config->swap_size;
}

t_muse_config* memoria_config_reload()
{
	memoria_config_free();
	memoria_config_load(memoria_log_get(), CONFIG_FILE_PATH, read_config, print_config);
	return memoria_config;
}

void read_config(t_config* config_file)
{
	memoria_config = malloc(sizeof(t_muse_config));
	memoria_config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
	memoria_config->memory_size = config_get_int_value(config_file, "MEMORY_SIZE");
	memoria_config->page_size = config_get_int_value(config_file, "PAGE_SIZE");
	memoria_config->swap_size = config_get_int_value(config_file, "SWAP_SIZE");
}

void print_config()
{
	memoria_logger_info("LISTEN_PORT: %d", memoria_config->listen_port);
	memoria_logger_info("MEMORY_SIZE: %d", memoria_config->memory_size);
	memoria_logger_info("PAGE_SIZE: %d", memoria_config->page_size);
	memoria_logger_info("SWAP_SIZE: %d", memoria_config->swap_size);
}
