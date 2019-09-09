#ifndef CONFIG_MEMORIA_CONFIG_H_
#define CONFIG_MEMORIA_CONFIG_H_

#include <stdbool.h>

struct setup_t {
	int listen_port;
	int memory_size;
	int page_size;
	int swap_size;
};

extern struct setup_t setup;

void init_config();

#endif /* MEMORIA_CONFIG_H_ */