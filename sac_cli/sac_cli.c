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


// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS
int main(int argc, char *argv[]) {
/* 	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

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

	// Esta es la funcion principal de FUSE
	return fuse_main(args.argc, args.argv, &sac_operations, NULL); */

	sac_cli_fd = socket_connect_to_server(ip_sac_server, puerto_sac);

	if (sac_cli_fd < 0) {
		printf("There was an error");
		socket_close_conection(sac_cli_fd);
		return -1;
	}

	printf("Conexion con SAC_SERVER establecida");
	t_read_dir *read_dir_send = malloc(sizeof(t_read_dir));
	read_dir_send->id_sac_cli = 123;
	read_dir_send->pathname = "/";
	t_protocol read_dir_protocol = READ_DIR;
	utils_serialize_and_send(sac_cli_fd, read_dir_protocol, read_dir_send);
	return 0;
}


int sac_create_directory(const char *path, mode_t mode) {
    return 0;
}

int sac_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    printf("Path %s: ", path);
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	// "." y ".." son entradas validas, la primera es una referencia al directorio donde estamos parados
	// y la segunda indica el directorio padre
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, "hello", NULL, 0);

	return 0;
}

int sac_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	size_t len;
	(void) fi;
	if (strcmp(path, DEFAULT_FILE_PATH) != 0)
		return -ENOENT;

	len = strlen(DEFAULT_FILE_CONTENT);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, DEFAULT_FILE_CONTENT + offset, size);
	} else
		size = 0;

	return size;
	return 0;
}

int sac_open(const char *path, struct fuse_file_info *fi) { 
	if (strcmp(path, DEFAULT_FILE_PATH) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

int sac_getattr(const char *path, struct stat *stbuf) { 
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));

	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, DEFAULT_FILE_PATH) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(DEFAULT_FILE_CONTENT);
	} else {
		res = -ENOENT;
	}
	
	return res;
}