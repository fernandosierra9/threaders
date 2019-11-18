#include "sac_server.h"

/*
 * 	Evalua si el ultimo caracter de str es chr.
 */
static int lastchar(const char* str, char chr){
	if ( ( str[strlen(str)-1]  == chr) ) return 1;
	return 0;
}

// Size del disco
int path_size(const char* path){
	FILE *fd;
	int size;
	fd=fopen(path, "r");
	fseek(fd, 0L, SEEK_END);
	size = ftell(fd);
	fclose(fd);
	return size;
}

/* @DESC
 * 		Determina cual es el nodo sobre el cual se encuentra un path.
 *
 * 	@PARAM
 * 		path - Direccion del directorio o archivo a buscar.
 *
 * 	@RETURN
 * 		Devuelve el numero de bloque en el que se encuentra el nombre.
 * 		Si el nombre no se encuentra, devuelve -1.
 *
 */
pointerSACBlock determinar_nodo(const char* path){

	// Si es el directorio raiz, devuelve 0:
	if (!strcmp(path, "/")) {
		return 0;
	}
	int i, nodo_anterior, err = 0;
	// Super_path usado para obtener la parte superior del path, sin el nombre.
	char *super_path = (char*) malloc(strlen(path)+1);
	char *nombre = (char*) malloc(strlen(path)+1);
	char *start = nombre;
	char *start_super_path = super_path;

	struct sac_file_t *node;
	unsigned char *node_name;

	split_path(path, &super_path, &nombre);
	nodo_anterior = determinar_nodo(super_path);
	//pthread_rwlock_rdlock(&rwlock); //Toma un lock de lectura.
	//log_lock_trace(logger, "Determinar_nodo: Toma lock lectura. Cantidad de lectores: %d", rwlock.__data.__nr_readers);

	node = node_table_start;
	// Busca el nodo sobre el cual se encuentre el nombre.
	node_name = &(node->file_name[0]);

	for (
	i = 0;
	((node->parent_dir_block != nodo_anterior) | (strcmp(nombre, (char*) node_name) != 0) | (node->state == 0)) &  (i < SAC_FILE_BY_TABLE);
	i++ ){
		node = &(node[1]);
		node_name = &(node->file_name[0]);
	}

	// Cierra conexiones y libera memoria. Contempla casos de error.
	//pthread_rwlock_unlock(&rwlock);
	//log_lock_trace(logger, "Determinar_nodo: Libera lock lectura. Cantidad de lectores: %d", rwlock.__data.__nr_readers);
	free(start);
	free(start_super_path);
	if (err != 0) return err;
	if (i >= SAC_FILE_BY_TABLE) return -1;
	return (i+1);
}

/*
 * 	DESC
 * 		Divide el path con formato de [RUTA] en: [RUTA_SUPERIOR] y [NOMBRE].
 * 		Ejemplo:
 * 			path: /home/utnso/algo.txt == /home/utnso - algo.txt
 * 			path: /home/utnso/ == /home - utnso
 *
 * 	PARAM
 * 		path - Ruta a dividir
 * 		super_path - Puntero sobre el cual se guardara la ruta superior.
 * 		name - Puntero al nombre del archivo
 *
 * 	RET
 * 		0 -> RTA OK
 *
 */
int split_path(const char* path, char** super_path, char** name){
	int aux;
	strcpy(*super_path, path);
	strcpy(*name, path);
	// Obtiene y acomoda el nombre del archivo.
	if (lastchar(path, '/')) {
		(*name)[strlen(*name)-1] = '\0';
	}
	*name = strrchr(*name, '/');
	*name = *name + 1; // Acomoda el nombre, ya que el primer digito siempre es '/'

	// Acomoda el super_path
	if (lastchar(*super_path, '/')) {
		(*super_path)[strlen(*super_path)-1] = '\0';
	}
	aux = strlen(*super_path) - strlen(*name);
	(*super_path)[aux] = '\0';

	return 0;
}

int get_size(void){
	return ((int) (ACTUAL_DISC_SIZE_B / BLOCK_SIZE));
}