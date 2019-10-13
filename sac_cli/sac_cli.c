#include "sac_cli.h"

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
		.mkdir = sac_create_directory,
		.write = sac_write,
		.rmdir = sac_rm_directory,
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

// ./sac_cli.exe -d -o direct_io --Disc-Path=/media/sf_tp-2019-2c-threaders/sac_server/disc.bin ./fuse_test
// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS
int main(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	// Limpio la estructura que va a contener los parametros
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		
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

	sac_cli_fd = socket_connect_to_server(ip_sac_server, puerto_sac);

	if (sac_cli_fd < 0) {
		printf("There was an error");
		socket_close_conection(sac_cli_fd);
		return -1;
	}

	printf("Conexion con SAC_SERVER establecida");
	
	// Esta es la funcion principal de FUSE
	return fuse_main(args.argc, args.argv, &sac_operations, NULL);
}

int sac_create_directory(const char *path, mode_t mode) {
    return 0;
};

int sac_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	t_read_dir *read_dir_send = malloc(sizeof(t_read_dir));
	read_dir_send->id_sac_cli = 123;
	read_dir_send->pathname = "/";
	t_protocol read_dir_protocol = READ_DIR;
	utils_serialize_and_send(sac_cli_fd, read_dir_protocol, read_dir_send);
	return 0;
};

int sac_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	t_read *read_send = malloc(sizeof(t_read));
	read_send->id_sac_cli = 456;
	read_send->pathname = "/";
	t_protocol read_protocol = READ;
	utils_serialize_and_send(sac_cli_fd, read_protocol, read_send);
	return 0;
};

int sac_open(const char *path, struct fuse_file_info *fi) { 
	t_open *open_send = malloc(sizeof(t_open));
	open_send->id_sac_cli = 789;
	open_send->pathname = "/";
	t_protocol open_protocol = OPEN;
	utils_serialize_and_send(sac_cli_fd, open_protocol, open_send);
	return 0;
};

int sac_getattr(const char *path, struct stat *stbuf) { 
	t_get_attr *get_attr_send = malloc(sizeof(t_get_attr));
	get_attr_send->id_sac_cli = 1011;
	get_attr_send->pathname = "/";
	t_protocol get_attr_protocol = GET_ATTR;
	utils_serialize_and_send(sac_cli_fd, get_attr_protocol, get_attr_send);
	return 0;
};

int sac_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	t_write *write_send = malloc(sizeof(t_write));
	write_send->id_sac_cli = 1112;
	write_send->pathname = "/";
	t_protocol write_protocol = WRITE;
	utils_serialize_and_send(sac_cli_fd, write_protocol, write_send);
	return 0;
};

int sac_rm_directory (const char* path) {
	t_rm_directory *rm_directory_send = malloc(sizeof(t_rm_directory));
	rm_directory_send->id_sac_cli = 1314;
	rm_directory_send->pathname = "/";
	t_protocol rm_dir_protocol = RM_DIR;
	utils_serialize_and_send(sac_cli_fd, rm_dir_protocol, rm_directory_send);
	return 0;
};