#include "sac_cli.h"


/* Local functions */
static int sac_cli_access(const char* path, int flags);
static int sac_cli_chmod(const char *path, mode_t mode);
static int sac_cli_chown(const char *path, uid_t user_data, gid_t group_data);

/*
 * Esta es una estructura auxiliar utilizada para almacenar parametros
 * que nosotros le pasemos por linea de comando a la funcion principal
 * de FUSE
 */
struct t_runtime_options {
	//char* disc_path;
} runtime_options;

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */

static struct fuse_operations sac_operations = {
		.getattr = sac_cli_getattr, // OK a medias
		.readdir = sac_cli_readdir,
		.open = sac_cli_open,
		.read = sac_cli_read,
		.mkdir = sac_cli_create_directory,
		.write = sac_cli_write,
		.rmdir = sac_cli_rm_directory,
		.access = sac_cli_access,		// OK
		.chmod = sac_cli_chmod,		// OK
		.chown = sac_cli_chown,		// OK

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
		// Estos son parametros por defecto que ya tiene FUSE
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_KEY("--version", KEY_VERSION),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_END,
};


// Dentro de los argumentos que recibe nuestro programa obligatoriamente
// debe estar el path al directorio donde vamos a montar nuestro FS
int sac_cli_init(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		perror("Invalid arguments!");
		return EXIT_FAILURE;
	}

	sac_cli_fd = socket_connect_to_server(ip_sac_server, puerto_sac);

	if (sac_cli_fd < 0) {
		printf("There was an error");
		socket_close_conection(sac_cli_fd);
		return -1;
	}

	printf("\n Conexion con SAC_SERVER establecida \n");
	return fuse_main(args.argc, args.argv, &sac_operations, NULL);
}

/*
 *  @ DESC
 * 		Funcion que crea directorios en el filesystem.
 * 	@ PARAM
 * 		-path: El path del directorio a crear
 * 		-mode: Contiene los permisos que debe tener el directorio y otra metadata
 * 	@ RET
 * 		0 si termino correctamente, negativo si hay error.
 */

int sac_cli_create_directory(const char *path, mode_t mode) {
	printf("\n SAC CLI: MAKE DIRECTORY, PATH: %s \n", path);
	int protocol;
	int server_response;
	int response;

	t_mk_directory* mk_directory_send = malloc(sizeof(t_mk_directory));
	mk_directory_send->pathname = strdup(path);
	mk_directory_send->id_sac_cli = 345;
	t_protocol mk_directory_protocol = MK_DIR;
	utils_serialize_and_send(sac_cli_fd, mk_directory_protocol, mk_directory_send);

	int received_bytes = recv(sac_cli_fd, &server_response, sizeof(int), 0);
	if (received_bytes <= 0){
		printf("Error al recibir la operacion del SAC_SERVER");
		printf("FIN");
		response = -ENOENT;
	}

	printf("\n MAKE DIRECTORY RESPONSE : %d\n", server_response);
	response = server_response;
	return response;
};


/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la lista de archivos o directorios que se encuentra dentro de un directorio
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es un buffer donde se colocaran los nombres de los archivos y directorios
 * 		      que esten dentro del directorio indicado por el path
 * 		filler - Este es un puntero a una función, la cual sabe como guardar una cadena dentro
 * 		         del campo buf
 *
 * 	@RETURN
 * 		O directorio fue encontrado. -ENOENT directorio no encontrado
 */

int sac_cli_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	printf("\n SAC CLI: READ DIRECTORY\n");
	t_read_dir *read_dir_send = malloc(sizeof(t_read_dir));
	read_dir_send->id_sac_cli = 123;
	read_dir_send->pathname = strdup(path);

	// "." y ".." obligatorios.
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	t_protocol read_dir_protocol = READ_DIR;
	utils_serialize_and_send(sac_cli_fd, read_dir_protocol, read_dir_send);

	int response = recv(sac_cli_fd, &read_dir_protocol, sizeof(t_protocol), 0);
	return 0;
};

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener el contenido de un archivo
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		buf - Este es el buffer donde se va a guardar el contenido solicitado
 * 		size - Nos indica cuanto tenemos que leer
 * 		offset - A partir de que posicion del archivo tenemos que leer
 *
 * 	@RETURN
 * 		Si se usa el parametro direct_io los valores de retorno son 0 si  elarchivo fue encontrado
 * 		o -ENOENT si ocurrio un error. Si el parametro direct_io no esta presente se retorna
 * 		la cantidad de bytes leidos o -ENOENT si ocurrio un error. ( Este comportamiento es igual
 * 		para la funcion write )
 */

int sac_cli_read(char *path) {
	printf("\n SAC CLI: READ\n");
	t_read *read_send = malloc(sizeof(t_read));
	read_send->id_sac_cli = 456;
	read_send->pathname = "/niconico";
	t_protocol read_protocol = READ;
	utils_serialize_and_send(sac_cli_fd, read_protocol, read_send);
	return 0;
};

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para tratar de abrir un archivo
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		fi - es una estructura que contiene la metadata del archivo indicado en el path
 *
 * 	@RETURN
 * 		O archivo fue encontrado. -EACCES archivo no es accesible
 */

int sac_cli_open(const char *path, struct fuse_file_info *fi) { 
	t_open *open_send = malloc(sizeof(t_open));
	open_send->id_sac_cli = 789;
	open_send->pathname = "/";
	t_protocol open_protocol = OPEN;
	utils_serialize_and_send(sac_cli_fd, open_protocol, open_send);
	return 0;
};

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la metadata de un archivo/directorio. Esto puede ser tamaño, tipo,
 * permisos, dueño, etc ...
 *
 * @PARAMETROS
 * 		path - El path es relativo al punto de montaje y es la forma mediante la cual debemos
 * 		       encontrar el archivo o directorio que nos solicitan
 * 		stbuf - Esta esta estructura es la que debemos completar
 *
 * 	@RETURN
 * 		O archivo/directorio fue encontrado. -ENOENT archivo/directorio no encontrado
 */
int sac_cli_getattr(const char *path, struct stat *stbuf) { 
	printf("\n SAC CLI: GET ATTR\n");

	int protocol;
	int res;
	int received_bytes;
	memset(stbuf, 0, sizeof(struct stat));

	/* 	if (strcmp(path, "/") == 0){
	  stbuf->st_mode = S_IFDIR | 0777;
	  stbuf->st_nlink = 2;
	  return 0;
	} */

	printf("\n UPDATED12 \n");
	
	t_get_attr *get_attr_send = malloc(sizeof(t_get_attr));
	get_attr_send->id_sac_cli = 1011;
	get_attr_send->pathname = strdup(path);
	t_protocol get_attr_protocol = GET_ATTR;
	utils_serialize_and_send(sac_cli_fd, get_attr_protocol, get_attr_send);
	received_bytes = recv(sac_cli_fd, &protocol, sizeof(int), 0);
	
	if (received_bytes <= 0){
		printf("\n Error al recibir la operacion del SAC_SERVER \n");
		printf("\n FIN \n");
		res = -ENOENT;
	}

	t_get_attr_server *get_attr_response = utils_receive_and_deserialize(sac_cli_fd, protocol);
	switch (protocol) {
		case GET_ATTR_RESPONSE: {
			printf("\n GET ATTRIBUTE OK \n");
			if (get_attr_response->state == 2){
				stbuf->st_mode = S_IFDIR | 0777;
				stbuf->st_nlink = 2;
				stbuf->st_size = 4096; // Default para los directorios, es una "convencion".
				stbuf->st_mtime = get_attr_response->modified_date;
				stbuf->st_ctime = get_attr_response->creation_date;
				stbuf->st_atime = time(NULL);
				res = 0;
			} else if (get_attr_response->state == 1){
				stbuf->st_mode = S_IFREG | 0777;
				stbuf->st_nlink = 1;
				stbuf->st_size = get_attr_response->file_size;
				stbuf->st_mtime = get_attr_response->modified_date;
				stbuf->st_ctime = get_attr_response->creation_date;
				stbuf->st_atime = time(NULL); 
				res = 0;
			}
			break;
		}
		default: {
			printf("\n DEFAULT \n");
			break;
		}
	}
	res = -ENOENT;
	return res;
};

int sac_cli_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	t_write *write_send = malloc(sizeof(t_write));
	write_send->id_sac_cli = 1112;
	write_send->pathname = "/";
	t_protocol write_protocol = WRITE;
	utils_serialize_and_send(sac_cli_fd, write_protocol, write_send);
	return 0;
};

int sac_cli_rm_directory (const char* path) {
	t_rm_directory *rm_directory_send = malloc(sizeof(t_rm_directory));
	rm_directory_send->id_sac_cli = 1314;
	rm_directory_send->pathname = "/";
	t_protocol rm_dir_protocol = RM_DIR;
	utils_serialize_and_send(sac_cli_fd, rm_dir_protocol, rm_directory_send);
	return 0;
};

static int sac_cli_access(const char* path, int flags){
	return 0;
}

static int sac_cli_chmod(const char *path, mode_t mode){
	return 0;
}

static int sac_cli_chown(const char *path, uid_t user_data, gid_t group_data){
	return 0;
}
