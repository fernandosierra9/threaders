#include "sac_cli.h"


/* Local functions */
static int sac_cli_access(const char* path, int flags);
static int sac_cli_chmod(const char *path, mode_t mode);
static int sac_cli_chown(const char *path, uid_t user_data, gid_t group_data);
static int sac_server_response(int *protocol);

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

// Buffers de write y read, que viajen por sockets
// agregar codigo para multithreading
// Falta descomentar los parametros en el server y testear real
// Falta arreglar problema que llega "pathname" con cualquier contenido (no tan grave)
// falta chequear el get_node de sac_operate porque falta una funcion de la commons
// testear bien todo y ver los memory leaks
static struct fuse_operations sac_operations = {
		.getattr = sac_cli_getattr, // SE LLAMA AL LEER METADATA DE UN ARCHIVO --> esta a medias
		.readdir = sac_cli_readdir, // LISTAR ARCHIVOS/DIRECTORIOS --> falta sac-cli que llegue la lista
		.mknod = sac_cli_mknod, // CREAR ARCHIVO, falta tester en sac-cli
		.unlink = sac_cli_unlink, // BORRAR ARCHIVO, falta testear en sac-cli
		.read = sac_cli_read, // LEER ARCHIVOS, falta pasar los buffer modificados por socket, testear en sac-cli
		.write = sac_cli_write, // ESCRIBIR ARCHIVO, falta pasar los buffer modificados por socket, testear en sac-cli
		.open = sac_cli_open, // SE LLAMA CUANDO SE ABRE UN ARCHIVO --> OK
		.mkdir = sac_cli_create_directory, // CREAR DIRECTORIO, falta testear en sac-cli
		.rmdir = sac_cli_rm_directory, // ELIMINAR DIRECTORIO, falta testear en sac-cli
		.access = sac_cli_access, // SETEA PERMISOS--> OK
		.chmod = sac_cli_chmod, // MODIFICA PERMISOS --> OK
		.chown = sac_cli_chown, // MODIFICA EL OWNER Y EL OWNER GROUP --> OK
		.flush = sac_cli_flush, // LIMPIA CACHE (creo que hay que borar este) --> OK
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
	int response;
	int protocol;

	t_mk_directory* mk_directory_send = malloc(sizeof(t_mk_directory));
	mk_directory_send->pathname = strdup(path);
	mk_directory_send->id_sac_cli = 345;
	t_protocol mk_directory_protocol = MK_DIR;
	utils_serialize_and_send(sac_cli_fd, mk_directory_protocol, mk_directory_send);

	int server_response = sac_server_response(&protocol);
	if (server_response == -1) return server_response;

	printf("\n MAKE DIRECTORY RESPONSE : %d\n", protocol);
	response = protocol;
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
	int protocol;
	t_read_dir *read_dir_send = malloc(sizeof(t_read_dir));
	read_dir_send->id_sac_cli = 123;
	read_dir_send->pathname = strdup(path);

	t_protocol read_dir_protocol = READ_DIR;
	utils_serialize_and_send(sac_cli_fd, read_dir_protocol, read_dir_send);

	int server_response = sac_server_response(&protocol);
	if (server_response == -1) return server_response;

	t_read_dir_server *read_dir_response = utils_receive_and_deserialize(sac_cli_fd, protocol);

	printf("\n READ DIRECTORY RESPONSE : %d\n", read_dir_response->res);
 	// "." y ".." obligatorios.
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	for (int j=0; j<list_size(read_dir_response->nodes); j++) {
		filler(buf, (char*) list_get(read_dir_response->nodes, j), NULL, 0);
	} 
	return read_dir_response->res;
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

int sac_cli_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
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
	printf("\n SAC CLI: OPEN\n");
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
	printf("\n SAC CLI: GET ATTRIBUTE, PATH: %s \n", path);
	int res;
	int protocol;
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0){
	  stbuf->st_mode = S_IFDIR | 0777;
	  stbuf->st_nlink = 2;
	  return 0;
	} 

	t_get_attr *get_attr_send = malloc(sizeof(t_get_attr));
	get_attr_send->id_sac_cli = 1011;
	get_attr_send->pathname = strdup(path);
	t_protocol get_attr_protocol = GET_ATTR;
	utils_serialize_and_send(sac_cli_fd, get_attr_protocol, get_attr_send);
	
	int server_response = sac_server_response(&protocol);
	if (server_response == -1) return server_response;

	t_get_attr_server *get_attr_response = utils_receive_and_deserialize(sac_cli_fd, protocol);

	if (get_attr_response->state == 2) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		stbuf->st_size = 4096; // Default para los directorios, es una "convencion".
		stbuf->st_mtime = get_attr_response->modified_date;
		stbuf->st_ctime = get_attr_response->creation_date;
		stbuf->st_atime = time(NULL);
		res = 0;
	} else if (get_attr_response->state == 1) {
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = get_attr_response->file_size;
		stbuf->st_mtime = get_attr_response->modified_date;
		stbuf->st_ctime = get_attr_response->creation_date;
		stbuf->st_atime = time(NULL); 
		res = 0;
	}

	res = -ENOENT;
	return res;
};


/*
 * 	@DESC
 * 		Funcion que escribe archivos en fuse.
 *
 * 	@PARAM
 * 		path - Dir del archivo
 * 		buf - Buffer que indica que datos copiar.
 * 		size - Tam de los datos a copiar
 * 		offset - Situa una posicion sobre la cual empezar a copiar datos
 * 		fi - File Info. Contiene flags y otras cosas locas que no hay que usar
 *
 * 	@RET
 * 		Devuelve la cantidad de bytes escritos, siempre y cuando este OK. Caso contrario, numero negativo tipo -ENOENT.
 */
int sac_cli_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	t_write *write_send = malloc(sizeof(t_write));
	write_send->id_sac_cli = 1112;
	write_send->pathname = strdup(path);
	write_send->buf = strdup(buf);
	write_send->size = size;
	write_send->offset = offset;
	t_protocol write_protocol = WRITE;
	utils_serialize_and_send(sac_cli_fd, write_protocol, write_send);
	return 0;
};

/*
 *	@DESC
 *		Funcion que borra directorios de fuse.
 *
 *	@PARAM
 *		Path - El path donde tiene que borrar.
 *
 *	@RET
 *		0 Si esta OK, -ENOENT si no pudo.
 *
 */
int sac_cli_rm_directory (const char* path) {
	printf("\n SAC CLI: REMOVE DIRECTORY, PATH: %s \n", path);
	int response;
	int protocol;
	t_rm_directory *rm_directory_send = malloc(sizeof(t_rm_directory));
	rm_directory_send->id_sac_cli = 1314;
	rm_directory_send->pathname = strdup(path);
	t_protocol rm_dir_protocol = RM_DIR;
	utils_serialize_and_send(sac_cli_fd, rm_dir_protocol, rm_directory_send);

	int server_response = sac_server_response(&protocol);
	if (server_response == -1) return server_response;

	printf("\n REMOVE DIRECTORY RESPONSE : %d\n", protocol);
	response = protocol;
	return response;
};


/*
 *  @DESC
 *  	Se invoca esta funcion cada vez que fuse quiere hacer un archivo nuevo
 *
 *  @PARAM
 *  	path - Como siempre, el path del archivo relativo al disco
 *  	mode - Opciones del archivo
 *  	dev - Otra cosa que no se usa :D
 *
 *  @RET
 *  	Devuelve 0 si le sale OK, num negativo si no.
 */
int sac_cli_mknod (const char* path, mode_t mode, dev_t dev) {
	printf("\n SAC CLI: CREATE NODE, PATH: %s \n", path);
	int response;
	int protocol;

	t_mk_node* mk_node_send = malloc(sizeof(t_mk_node));
	mk_node_send->pathname = strdup(path);
	mk_node_send->id_sac_cli = 445;
	t_protocol mk_node_protocol = MK_NODE;
	utils_serialize_and_send(sac_cli_fd, mk_node_protocol, mk_node_send);

	int server_response = sac_server_response(&protocol);
	if (server_response == -1) return server_response;

	printf("\n CREATE NODE RESPONSE : %d\n", protocol);
	response = protocol;
	return response;
}

/*
 *  @DESC
 *  	Funcion que se llama cuando hay que borrar un archivo
 *
 *  @PARAM
 *  	path - La ruta del archivo a borrar.
 *
 *  @RET
 *  	0 si salio bien
 *  	Numero negativo, si no
 */

int sac_cli_unlink (const char* path) {
	printf("\n SAC CLI: DELETE NODE, PATH: %s \n", path);
	int response;
	int protocol;

	t_unlink_node* unlink_node_send = malloc(sizeof(t_unlink_node));
	unlink_node_send->pathname = strdup(path);
	unlink_node_send->id_sac_cli = 143;
	t_protocol unlink_node_protocol = UNLINK_NODE;
	utils_serialize_and_send(sac_cli_fd, unlink_node_protocol, unlink_node_send);

	int server_response = sac_server_response(&protocol);
	if (server_response == -1) return server_response;

	printf("\n DELETE NODE RESPONSE : %d\n", protocol);
	response = protocol;
	return response;
}

int sac_cli_flush(const char* path, struct fuse_file_info *fi){
	printf("\n SAC CLI: FLUSH, PATH: %s \n", path);
	int response;
	int protocol;

	t_flush* flush_send = malloc(sizeof(t_flush));
	flush_send->pathname = strdup(path);
	flush_send->id_sac_cli = 565;
	t_protocol flush_protocol = FLUSH;
	utils_serialize_and_send(sac_cli_fd, flush_protocol, flush_send);

	int server_response = sac_server_response(&protocol);
	if (server_response == -1) return server_response;

	printf("\n FLUSH RESPONSE : %d\n", protocol);
	response = protocol;
	return response;

}


/*
 *  DESC
 *  	Settea los permisos de acceso a un file
 *
 *  PARAM
 *  	path - path del archivo
 *  	flags - flags que corresponden a los permisos del archivo
 *
 *  RET
 *  	0 - Access granted
 *  	-1 - Access denied
 */

static int sac_cli_access(const char* path, int flags){
	return 0;
}

/*
 * 	@DESC
 * 		Modifica los permisos del archivo.
 * 		Como nosotros no trabajamos con permisos en el FS, esta funcion sera simplemente un hermoso Dummy.
 *
 * 	@PARAM
 * 		path - Ruta del archivo a cambiarle permisos
 * 		mode - Estructura que contiene los datos a cambiar del archivo
 *
 * 	@RETURN
 * 		0 - Funciona.
 * 		Negativo - Rompe.
 */

static int sac_cli_chmod(const char *path, mode_t mode){
	return 0;
}

/*
 * 	@DESC
 * 		Modifica el owner y owner group del archivo.
 *
 * 	@PARAM
 * 		path - Ruta del archivo a cambiarle permisos
 *		user_data - Datos del usuario
 *		group_data - Datos del grupo
 *
 * 	@RETURN
 * 		0 - Funciona.
 * 		Negativo - Rompe.
 */
static int sac_cli_chown(const char *path, uid_t user_data, gid_t group_data){
	return 0;
}

static int sac_server_response(int *protocol) {
	int response = 0;
	int received_bytes = recv(sac_cli_fd, protocol, sizeof(int), 0);
	if (received_bytes <= 0){
		printf("\n Error al recibir la operacion del SAC_SERVER \n");
		printf("\n Fin de SAC_CLI \n");
		response = -1;
	}
	return response;
}
