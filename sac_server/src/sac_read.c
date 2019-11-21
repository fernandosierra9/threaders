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

int sac_server_read(const char *path, char *buf, size_t size, off_t offset) {
	sac_server_logger_info("Read: Path: %s ", path);
	//(void) fi;
	unsigned int nodo  =determinar_nodo(path);
	unsigned int bloque_punteros, num_bloque_datos, bloque_a_buscar;
	struct sac_file_t *node;
	pointerSACBlock *pointer_block;
	char *data_block;
	size_t tam = size;
	int res;

	if (nodo == -1) return -ENOENT;

	node = node_table_start;

	// Ubica el nodo correspondiente al archivo
	node = &(node[nodo-1]);

	//pthread_rwlock_rdlock(&rwlock); //Toma un lock de lectura.
	//log_lock_trace(logger, "Read: Toma lock lectura. Cantidad de lectores: %d", rwlock.__data.__nr_readers);

	if(node->file_size <= offset){
		sac_server_logger_info("Fuse intenta leer un offset mayor o igual que el tamanio de archivo. Se retorna size 0. File: %s, Size: %d", path, node->file_size);
		res = 0;
		goto finalizar;
	} else if (node->file_size <= (offset+size)){
		tam = size = ((node->file_size)-(offset));
		sac_server_logger_info("Fuse intenta leer un offset mayor o igual que el tamanio de archivo. Se retorna size 0. File: %s, Size: %d", path, node->file_size);
	}
	// Recorre todos los punteros en el bloque de la tabla de nodos
	for (bloque_punteros = 0; bloque_punteros < BLOCK_INDIRECT; bloque_punteros++){
		// Chequea el offset y lo acomoda para leer lo que realmente necesita
		if (offset > BLOCK_SIZE * 1024){
			offset -= (BLOCK_SIZE * 1024);
			continue;
		}

		bloque_a_buscar = (node->block_indirect)[bloque_punteros];	// Ubica el nodo de punteros a nodos de datos, es relativo al nodo 0: Header.
		bloque_a_buscar -= (SAC_FILE_BY_BLOCK + BITMAP_BLOCK_SIZE + NODE_TABLE_SIZE);	// Acomoda el nodo de punteros a nodos de datos, es relativo al bloque de datos.
		pointer_block =(pointerSACBlock *) &(data_block_start[bloque_a_buscar]);		// Apunta al nodo antes ubicado. Lo utiliza para saber de donde leer los datos.

		// Recorre el bloque de punteros correspondiente.
		for (num_bloque_datos = 0; num_bloque_datos < 1024; num_bloque_datos++){

			// Chequea el offset y lo acomoda para leer lo que realmente necesita
			if (offset >= BLOCK_SIZE){
				offset -= BLOCK_SIZE;
				continue;
			}

			bloque_a_buscar = pointer_block[num_bloque_datos]; 	// Ubica el nodo de datos correspondiente. Relativo al nodo 0: Header.
			bloque_a_buscar -= (SAC_FILE_BY_BLOCK + BITMAP_BLOCK_SIZE + NODE_TABLE_SIZE);	// Acomoda el nodo, haciendolo relativo al bloque de datos.
			data_block = (char *) &(data_block_start[bloque_a_buscar]);

			// Corre el offset hasta donde sea necesario para poder leer lo que quiere.
			if (offset > 0){
				data_block += offset;
				offset = 0;
			}

			if (tam < BLOCK_SIZE){
				memcpy(buf, data_block, tam);
				buf = &(buf[tam]);
				tam = 0;
				break;
			} else {
				memcpy(buf, data_block, BLOCK_SIZE);
				tam -= BLOCK_SIZE;
				buf = &(buf[BLOCK_SIZE]);
				if (tam == 0) break;
			}

		}

		if (tam == 0) break;
	}
	res = size;

	finalizar:
	//pthread_rwlock_unlock(&rwlock); //Devuelve el lock de lectura.
	//log_lock_trace(logger, "Read: Libera lock lectura. Cantidad de lectores: %d", rwlock.__data.__nr_readers);
	//log_trace(logger, "Terminada lectura.");
	return res;
}