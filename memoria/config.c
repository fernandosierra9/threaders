#include <stdbool.h>
#include <stdlib.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

#include "memoria.h"
#include "config.h"

/* -- Local function prototypes  -- */
static void check_config(t_config* config, char* key);

/* -- Global variables -- */
t_log *logger;
struct setup_t setup;

void init_config() {
    t_config* config = config_create("./configs/memoria.config");
	log_info(logger, "Se abrio el archivo de configuracion.");

	check_config(config, "LISTEN_PORT");
	setup.listen_port = config_get_int_value(config, "LISTEN_PORT");
	log_info(logger, "Asignado el valor del puerto de escucha %d.", setup.listen_port);

	check_config(config, "MEMORY_SIZE");
	setup.memory_size = config_get_int_value(config, "MEMORY_SIZE");
	log_info(logger, "Asignada el tamaño de memoria de %d bytes.", setup.memory_size);

	check_config(config, "PAGE_SIZE");
	setup.page_size = config_get_int_value(config, "PAGE_SIZE");
	log_info(logger, "Asignado el tamaño de pagina de %d bytes.", setup.page_size);

	check_config(config, "SWAP_SIZE");
	setup.swap_size = config_get_int_value(config, "SWAP_SIZE");
	log_info(logger, "Asignado el tamaño del swap %d.", setup.swap_size);

	log_info(logger, "Se configuro el Coordinador correctamente.");

	config_destroy(config);
}

static void check_config(t_config *config, char* key) {
	if (!config_has_property(config, key)) {
		log_error(logger, "No existe la clave '%s' en el archivo de configuracion.", key);
		exit_gracefully(EXIT_FAILURE);
	}
}