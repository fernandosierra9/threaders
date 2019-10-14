#include "sac_server.h"

/*
 *  @ DESC
 * 		Funcion que crea directorios en el filesystem.
 * 	@ PARAM
 * 		-path: El path del directorio a crear
 * 		-mode: Contiene los permisos que debe tener el directorio y otra metadata
 * 	@ RET
 * 		0 si termino correctamente, negativo si hay error.
 */
int sac_create_directory (const char *path, mode_t mode){
    sac_server_logger_info("Making directory, Path: %s", path);
	int father_node, i, res = 0;
	struct sac_file_t* node;
	char *nombre = malloc(strlen(path) + 1), *nom_to_free = nombre;
	char *dir_padre = malloc(strlen(path) + 1), *dir_to_free = dir_padre;

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
	for (i = 0; (node->state != 0) & (i <= NODE_TABLE_SIZE); i++) node = &(node_table_start[i]);
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