#include "sac_server.h"

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
int sac_server_getattr(const char *path, struct stat *stbuf) {
	sac_server_logger_info("Getattr: Path: %s", path);
	int nodo = determinar_nodo(path), res;
	if (nodo < 0) return -ENOENT;
	struct sac_file_t *node;

	if (nodo == -1) return -ENOENT;

	if (strcmp(path, "/") == 0){
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		return 0;
	}

	node = node_table_start;

	node = &(node[nodo-1]);

	if (node->state == 2){
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
		stbuf->st_size = 4096; // Default para los directorios, es una "convencion".
		stbuf->st_mtime = node->modified_date;
		stbuf->st_ctime = node->creation_date;
		stbuf->st_atime = time(NULL);
		res = 0;
		goto finalizar;
	} else if(node->state == 1){
		stbuf->st_mode = S_IFREG | 0777;
		stbuf->st_nlink = 1;
		stbuf->st_size = node->file_size;
		stbuf->st_mtime = node->modified_date;
		stbuf->st_ctime = node->creation_date;
		stbuf->st_atime = time(NULL); 
		res = 0;
		goto finalizar;
	}

	res = -ENOENT;
	// Cierra conexiones y libera memoria.
	finalizar:
	return res; 
}

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
int sac_server_open(const char *path, struct fuse_file_info *fi) {
	if (strcmp(path, DEFAULT_FILE_PATH) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

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

int sac_server_readdir(const char *path) {
	sac_server_logger_info("Readdir: Path: %s - Offset %d", path, offset);
	int i;
	int res = 0;
	int nodo = determinar_nodo(path);
	struct sac_file_t *node;

	if (nodo == -1) return  -ENOENT;

	node = node_table_start;

	// "." y ".." obligatorios.
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	//pthread_rwlock_rdlock(&rwlock); //Toma un lock de lectura.
	//log_lock_trace(logger, "Readdir: Toma lock lectura. Cantidad de lectores: %d", rwlock.__data.__nr_readers);

	// Carga los nodos que cumple la condicion en el buffer.
	for (i = 0; i < SAC_FILE_BY_TABLE;  (i++)){
		if ((nodo==(node->parent_dir_block)) & (((node->state) == DIRECTORY_T) | ((node->state) == FILE_T))) 
		filler(buf, (char*) &(node->file_name[0]), NULL, 0);
		node = &node[1];
	}

	//pthread_rwlock_unlock(&rwlock); //Devuelve un lock de lectura.
	//log_lock_trace(logger, "Readdir: Libera lock lectura. Cantidad de lectores: %d", rwlock.__data.__nr_readers);
	return res;
}

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
int sac_server_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
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
}