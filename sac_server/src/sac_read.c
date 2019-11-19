#include "sac_server.h"


int sac_server_getattr(const char *path, struct sac_file_t *node) {
	sac_server_logger_info("Getattr: Path: %s", path);
	int nodo = determinar_nodo(path);
	int res;
	
	if (nodo < 0) {
		res = -ENOENT;
	} else {
		node = node_table_start;
		node = &(node[nodo-1]);
		res = 0;
	}

	return res; 
}

int sac_server_open(const char *path, struct fuse_file_info *fi) {
	if (strcmp(path, DEFAULT_FILE_PATH) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

int sac_server_readdir(const char *path, t_list* nodes) {
	sac_server_logger_info("Readdir: Path: %s ", path);
	int i;
	int res = 0;
	int nodo = determinar_nodo(path);
	struct sac_file_t *node;

	if (nodo == -1) return  -ENOENT;
	
	node = node_table_start;
	//pthread_rwlock_rdlock(&rwlock); //Toma un lock de lectura.
	//log_lock_trace(logger, "Readdir: Toma lock lectura. Cantidad de lectores: %d", rwlock.__data.__nr_readers);

	// Carga los nodos que cumple la condicion en el buffer.
	for (i = 0; i < SAC_FILE_BY_TABLE;  (i++)){
		if ((nodo==(node->parent_dir_block)) & (((node->state) == DIRECTORY_T) | ((node->state) == FILE_T))) {
			list_add(nodes, &(node->file_name[0]));
		}
		node = &node[1];
	}
	
	//pthread_rwlock_unlock(&rwlock); //Devuelve un lock de lectura.
	//log_lock_trace(logger, "Readdir: Libera lock lectura. Cantidad de lectores: %d", rwlock.__data.__nr_readers);
	return res;
}

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