#include "sac_server.h"

int sac_server_create_directory (const char *path){
    sac_server_logger_info("Making directory, Path: %s", path);
	int father_node, i, res = 0;
	struct sac_file_t* node;
	char *nombre = malloc(strlen(path) + 1);
	char *nom_to_free = nombre;
	char *dir_padre = malloc(strlen(path) + 1);
	char *dir_to_free = dir_padre;

	if (determinar_nodo(path) != -1) return -EEXIST;

	split_path(path, &dir_padre, &nombre);

	// Ubica el nodo correspondiente. Si es el raiz, lo marca como 0. Si no existe, lo informa.
	if (strcmp(dir_padre, "/") == 0){
		father_node = 0;
	} else if ((father_node = determinar_nodo(dir_padre)) < 0){
		return -ENOENT;
	}

	node = node_table_start;

	// Toma un lock de escritura.
	// log_lock_trace(logger, "Mkdir: Pide lock escritura. Escribiendo: %d. En cola: %d.", rwlock.__data.__writer, rwlock.__data.__nr_writers_queued);
	// pthread_rwlock_wrlock(&rwlock);
	// log_lock_trace(logger, "Mkdir: Recibe lock escritura.");
	// Abrir conexion y traer directorios, guarda el bloque de inicio para luego liberar memoria

	// Busca el primer nodo libre (state 0) y cuando lo encuentra, lo crea:
	for (i = 0; (node->state != 0) & (i <= NODE_TABLE_SIZE); i++) {
		node = &(node_table_start[i]);
	}
	// Si no hay un nodo libre, devuelve un error.
	if (i > NODE_TABLE_SIZE){
		res = -EDQUOT;
		goto finalizar;
	}
	// Escribe datos del archivo
	node->state = DIRECTORY_T;
	strcpy((char*) &(node->file_name[0]), nombre);
	node->file_size = 0;
	node->parent_dir_block = father_node;
	res = 0;

	finalizar:
	free(nom_to_free);
	free(dir_to_free);

	// Devuelve el lock de escritura.
	// pthread_rwlock_unlock(&rwlock);
	// log_lock_trace(logger, "Mkdir: Devuelve lock escritura. En cola: %d", rwlock.__data.__nr_writers_queued);
	return res;

}

int sac_server_remove_directory(const char* path) {
	sac_server_logger_info("Remove directory, Path: %s", path);
	int nodo_padre = determinar_nodo(path);
	int i;
	int res = 0;
	if (nodo_padre == -1) return -ENOENT;
	struct sac_file_t *node;

	//log_lock_trace(logger, "Rmdir: Pide lock escritura. Escribiendo: %d. En cola: %d.", rwlock.__data.__writer, rwlock.__data.__nr_writers_queued);
	//pthread_rwlock_wrlock(&rwlock);
	//log_lock_trace(logger, "Rmdir: Recibe lock escritura.");

	// Abre conexiones y levanta la tabla de nodos en memoria.
	node = &(node_table_start[-1]);
	node = &(node[nodo_padre]);

	// Chequea si el directorio esta vacio. En caso que eso suceda, FUSE se encarga de borrar lo que hay dentro.
	for (i=0; i < 1024 ;i++){
		if (((&node_table_start[i])->state != DELETED_T) & ((&node_table_start[i])->parent_dir_block == nodo_padre)) {
			res = -ENOTEMPTY;
			goto finalizar;
		}
	}

	node->state = DELETED_T; // Aca le dice que el estado queda "Borrado"

	finalizar:
	// Devuelve el lock de escritura.
	//pthread_rwlock_unlock(&rwlock);
	//log_lock_trace(logger, "Rmdir: Devuelve lock escritura. En cola: %d", rwlock.__data.__nr_writers_queued);
	return res;
}


/*
 *
 */
int sac_server_flush(){

	// Toma un lock de escritura.
	//log_lock_trace(logger, "Flush: Pide lock escritura. Escribiendo: %d. En cola: %d.", rwlock.__data.__writer, rwlock.__data.__nr_writers_queued);
	//pthread_rwlock_wrlock(&rwlock);
	//log_lock_trace(logger, "Flush: Recibe lock escritura.");
	fdatasync(discDescriptor);
	// Devuelve el lock de escritura.
	//pthread_rwlock_unlock(&rwlock);
	//log_lock_trace(logger, "Flush: Devuelve lock escritura. En cola: %d", rwlock.__data.__nr_writers_queued);
	return 0;
}
