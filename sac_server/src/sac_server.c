#include "sac_server.h"

/*
 * Esta es una estructura auxiliar utilizada para almacenar parametros
 * que nosotros le pasemos por linea de comando a la funcion principal
 * de FUSE
 */
struct t_runtime_options {
	char* disc_path;
} runtime_options;

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */

static struct fuse_operations sac_operations = {
		.getattr = sac_getattr,
		.readdir = sac_readdir,
		.open = sac_open,
		.read = sac_read,
};

/** keys for FUSE_OPT_ options */
enum {
	KEY_VERSION,
	KEY_HELP,
};

/*
 * Esta estructura es utilizada para decirle a la biblioteca de FUSE que
 * parametro puede recibir y donde tiene que guardar el valor de estos
 */
static struct fuse_opt fuse_options[] = {
		// Si se le manda el parametro "--Disc-Path", lo utiliza:
		CUSTOM_FUSE_OPT_KEY("--Disc-Path=%s", disc_path, 0),
		// Estos son parametros por defecto que ya tiene FUSE
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_KEY("--version", KEY_VERSION),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_END,
};


// Init  ./sac_server.exe -d -o direct_io -s ./fuse_test
// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS
int main(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	int res, fd;
	// Limpio la estructura que va a contener los parametros
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		/** error parsing options */
		perror("Invalid arguments!");
		return EXIT_FAILURE;
	}

	// Setea el path del disco
	if (runtime_options.disc_path != NULL){
		strcpy(fuse_disc_path, runtime_options.disc_path);
	} else {
		printf("Mountpoint not specified: Unloading modules.");
		exit(0);
	}

	// Creo logger y leo config
	sac_server_logger_create();
	sac_server_config_load();

	// Obiene el tamanio del disco
	fuse_disc_size = path_size(DISC_PATH);

	printf("\n DISC SIZE %d\n", fuse_disc_size);

	/*
	BitArray
	*/

	if ((fd = open(DISC_PATH, O_RDWR, 0)) == -1) {
		sac_server_logger_error("Error");
	}
	
	header_start = (struct sac_header_t*) mmap(NULL, ACTUAL_DISC_SIZE_B , PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
	header_data = *header_start;
	bitmap_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS];
	node_table_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS + BITMAP_BLOCK_SIZE];
	data_block_start = (struct sac_file_t*) &header_start[SAC_HEADER_BLOCKS + BITMAP_BLOCK_SIZE + NODE_TABLE_SIZE];
	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads
	res = fuse_main(args.argc, args.argv, &sac_operations, NULL);

	sac_server_logger_destroy();

	return res;
}
