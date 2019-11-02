#include "muse_config.h"

t_muse_config* muse_config_reload();
void read_config(t_config* config_file);
void print_config();

int muse_config_load()
{
	muse_logger_info("Se establecerá la configuración");
	return config_load(muse_log_get(), CONFIG_FILE_PATH, read_config, print_config);
}

void muse_config_free()
{
	free(muse_config);
}

int muse_get_listen_port()
{
	return muse_config->listen_port;
}

int muse_memory_size()
{
	return muse_config->memory_size;
}

int muse_page_size()
{
	return muse_config->page_size;
}

int muse_swap_size()
{
	return muse_config->swap_size;
}


t_muse_config* muse_config_reload()
{
	muse_config_free();
	muse_config_load(muse_log_get(), CONFIG_FILE_PATH, read_config, print_config);
	return muse_config;
}

void read_config(t_config* config_file)
{
	muse_config = malloc(sizeof(t_muse_config));
	muse_config->listen_port = config_get_int_value(config_file, "LISTEN_PORT");
	muse_config->memory_size = config_get_int_value(config_file, "MEMORY_SIZE");
	muse_config->page_size = config_get_int_value(config_file, "PAGE_SIZE");
	muse_config->swap_size = config_get_int_value(config_file, "SWAP_SIZE");

}

void print_config()
{
	muse_logger_info("LISTEN_PORT: %d", muse_config->listen_port);
	muse_logger_info("MEMORY_SIZE: %d", muse_config->memory_size);
	muse_logger_info("PAGE_SIZE: %d", muse_config->page_size);
	muse_logger_info("SWAP_SIZE: %d", muse_config->swap_size);
}
