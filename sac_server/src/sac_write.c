#include "sac_server.h"



int sac_server_create_directory (const char *path){
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


int sac_server_make_node(const char* path) {
	if (determinar_nodo(path) != -1) return -EEXIST;
	int nodo_padre, i, res = 0;
	int new_free_node;
	struct sac_file_t *node;
	char *nombre = malloc(strlen(path) + 1);
	char *nom_to_free = nombre;
	char *dir_padre = malloc(strlen(path) + 1);
	char *dir_to_free = dir_padre;
	char *data_block;

	split_path(path, &dir_padre, &nombre);

	// Ubica el nodo correspondiente. Si es el raiz, lo marca como 0, Si es menor a 0, lo crea (mismos permisos).
	if (strcmp(dir_padre, "/") == 0) nodo_padre = 0;
	else if ((nodo_padre = determinar_nodo(dir_padre)) < 0) return -ENOENT;
	node = node_table_start;

	// Toma un lock de escritura.
	//log_lock_trace(logger, "Mknod: Pide lock escritura. Escribiendo: %d. En cola: %d.", rwlock.__data.__writer, rwlock.__data.__nr_writers_queued);
	//pthread_rwlock_wrlock(&rwlock);
	//log_lock_trace(logger, "Mknod: Recibe lock escritura.");

	// Busca el primer nodo libre (state 0) y cuando lo encuentra, lo crea:
	for (i = 0; (node->state != 0) & (i <= NODE_TABLE_SIZE); i++) node = &(node_table_start[i]);
	// Si no hay un nodo libre, devuelve un error.
	if (i > NODE_TABLE_SIZE){
		res = -EDQUOT;
		goto finalizar;
	}

	// Escribe datos del archivo
	node->state = FILE_T;
	strcpy((char*) &(node->file_name[0]), nombre);
	node->file_size = 0; // El tamanio se ira sumando a medida que se escriba en el archivo.
	node->parent_dir_block = nodo_padre;
	node->block_indirect[0] = 0; // Se utiliza esta marca para avisar que es un archivo nuevo. De esta manera, la funcion add_node conoce que esta recien creado.
	node->creation_date = node->modified_date = time(NULL);
	res = 0;

	// Obtiene un bloque libre para escribir.
	new_free_node = get_node();
	sac_server_logger_info("new_free_node %d", new_free_node);
	// Actualiza la informacion del archivo.
	add_node(node, new_free_node);

	// Lo relativiza al data block.
	new_free_node -= (SAC_FILE_BY_BLOCK + NODE_TABLE_SIZE + BITMAP_BLOCK_SIZE);
	data_block = (char*) &(data_block_start[new_free_node]);

	// Escribe en ese bloque de datos.
	memset(data_block, '\0', BLOCK_SIZE);

	finalizar:
	free(nom_to_free);
	free(dir_to_free);

	// Devuelve el lock de escritura.
	//pthread_rwlock_unlock(&rwlock);
	//log_lock_trace(logger, "Mknod: Devuelve lock escritura. En cola: %d", rwlock.__data.__nr_writers_queued);
	return res;
}

int sac_server_unlink_node(const char* path) {
	struct sac_file_t* file_data;
	int node = determinar_nodo(path);

	ENABLE_DELETE_MODE;

	file_data = &(node_table_start[node - 1]);

	// Toma un lock de escritura.
	// log_lock_trace(logger, "Ulink: Pide lock escritura. Escribiendo: %d. En cola: %d.", rwlock.__data.__writer, rwlock.__data.__nr_writers_queued);
	// pthread_rwlock_wrlock(&rwlock);
	// log_lock_trace(logger, "Ulink: Recibe lock escritura.");

	delete_nodes_upto(file_data, 0, 0);

	// Devuelve el lock de escritura.
	// pthread_rwlock_unlock(&rwlock);
	// log_lock_trace(logger, "Ulink: Devuelve lock escritura. En cola: %d", rwlock.__data.__nr_writers_queued);
	DISABLE_DELETE_MODE;

	return sac_server_remove_directory(path);
}

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

int sac_server_write(const char *path, const char *buf, size_t size, off_t offset) {
	sac_server_logger_info("Write node, Path: %s", path);
	int nodo = determinar_nodo(path);
	if (nodo == -1) return -ENOENT;
	int new_free_node;
	struct sac_file_t *node;
	char *data_block;
	size_t tam = size, file_size, space_in_block, offset_in_block = offset % BLOCK_SIZE;
	off_t off = offset;
	int *n_pointer_block = malloc(sizeof(int)), *n_data_block = malloc(sizeof(int));
	pointerSACBlock *pointer_block;
	int res = size;

	// Ubica el nodo correspondiente al archivo
	node = &(node_table_start[nodo-1]);
	file_size = node->file_size;

	if ((file_size + size) >= THELARGESTFILE) return -EFBIG;

	// Toma un lock de escritura.
	//log_lock_trace(logger, "Write: Pide lock escritura. Escribiendo: %d. En cola: %d.", rwlock.__data.__writer, rwlock.__data.__nr_writers_queued);
	//pthread_rwlock_wrlock(&rwlock);
	//log_lock_trace(logger, "Write: Recibe lock escritura.");

	// Guarda tantas veces como sea necesario, consigue nodos y actualiza el archivo.
	while (tam != 0){

		// Actualiza los valores de espacio restante en bloque.
		space_in_block = BLOCK_SIZE - (file_size % BLOCK_SIZE);
		if (space_in_block == BLOCK_SIZE) (space_in_block = 0); // Porque significa que el bloque esta lleno.
		if (file_size == 0) space_in_block = BLOCK_SIZE; /* Significa que el archivo esta recien creado y ya tiene un bloque de datos asignado */

		// Si el offset es mayor que el tamanio del archivo mas el resto del bloque libre, significa que hay que pedir un bloque nuevo
		// file_size == 0 indica que es un archivo que recien se comienza a escribir, por lo que tiene un tratamiento distinto (ya tiene un bloque de datos asignado).
		if ((off >= (file_size + space_in_block)) & (file_size != 0)){

			// Si no hay espacio en el disco, retorna error.
			if (bitmap_free_blocks == 0) return -ENOSPC;

			// Obtiene un bloque libre para escribir.
			new_free_node = get_node();
			if (new_free_node < 0) goto finalizar;

			// Agrega el nodo al archivo.
			res = add_node(node, new_free_node);
			if (res != 0) goto finalizar;

			// Lo relativiza al data block.
			new_free_node -= (SAC_HEADER_BLOCKS + NODE_TABLE_SIZE + BITMAP_BLOCK_SIZE);
			data_block = (char*) &(data_block_start[new_free_node]);

			// Actualiza el espacio libre en bloque.
			space_in_block = BLOCK_SIZE;

		} else {
			// Ubica a que nodo le corresponderia guardar el dato
			set_position(n_pointer_block, n_data_block, file_size, off);

			//Ubica el nodo a escribir.
			*n_pointer_block = node->block_indirect[*n_pointer_block];
			*n_pointer_block -= (SAC_HEADER_BLOCKS + NODE_TABLE_SIZE + BITMAP_BLOCK_SIZE);
			pointer_block = (pointerSACBlock*) &(data_block_start[*n_pointer_block]);
			*n_data_block = pointer_block[*n_data_block];
			*n_data_block -= (SAC_HEADER_BLOCKS + NODE_TABLE_SIZE + BITMAP_BLOCK_SIZE);
			data_block = (char*) &(data_block_start[*n_data_block]);
		}

		// Escribe en ese bloque de datos.
		if (tam >= BLOCK_SIZE){
			memcpy(data_block, buf, BLOCK_SIZE);
			if ((node->file_size) <= (off)) file_size = node->file_size += BLOCK_SIZE;
			buf += BLOCK_SIZE;
			off += BLOCK_SIZE;
			tam -= BLOCK_SIZE;
			offset_in_block = 0;
		} else if (tam <= space_in_block){ /*Hay lugar suficiente en ese bloque para escribir el resto del archivo */
			memcpy(data_block + offset_in_block, buf, tam);
			if (node->file_size <= off) file_size = node->file_size += tam;
			else if (node->file_size <= (off + tam)) file_size = node->file_size += (off + tam - node->file_size);
			tam = 0;
		} else { /* Como no hay lugar suficiente, llena el bloque y vuelve a buscar uno nuevo */
			memcpy(data_block + offset_in_block, buf, space_in_block);
			file_size = node->file_size += space_in_block;
			buf += space_in_block;
			off += space_in_block;
			tam -= space_in_block;
			offset_in_block = 0;
		}

	}

	node->modified_date= time(NULL);

	res = size;

	finalizar:
	// Devuelve el lock de escritura.
	//pthread_rwlock_unlock(&rwlock);
	//log_lock_trace(logger, "Write: Devuelve lock escritura. En cola: %d", rwlock.__data.__nr_writers_queued);
	//log_trace(logger, "Terminada escritura.");
	return res;
}