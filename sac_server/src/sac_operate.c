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

/*
 *	@DESC
 *		Borra los nodos hasta la estructura correspondiente (upto:hasta) especificadas. EXCLUSIVE.
 *
 *	@PARAM
 *
 *
 *	@RET
 *
 */
int delete_nodes_upto (struct sac_file_t *file_data, int pointer_upto, int data_upto){
	t_bitarray *bitarray;
	size_t file_size = file_data->file_size;
	int node_to_delete, node_pointer_to_delete, delete_upto;
	pointerSACBlock *aux; // Auxiliar utilizado para saber que nodo redireccionar
	int data_pos, pointer_pos;

	// Ubica cual es el ultimo nodo del archivo
	set_position(&pointer_pos, &data_pos, 0, file_size);
	if (file_size%(BLOCK_SIZE*PTR_BLOCK_SIZE) == 0) {
		pointer_pos--;
		data_pos = PTR_BLOCK_SIZE-1;
	}
	else if (file_size%BLOCK_SIZE == 0) data_pos--;

	// Crea el bitmap
	bitarray = bitarray_create((char*) bitmap_start, BITMAP_SIZE_B);

	// Activa el DELETE_MODE. Este modo NO debe activarse cuando se hacen operaciones que
	// dejen al archivo con un solo nodo. Por ejemplo, truncate -s 0.
	// Deberia estar activo en otro momento.
	if((pointer_upto != 0) | (data_upto != 0)) DISABLE_DELETE_MODE;

	// Borra hasta que los nodos de posicion coincidan con los nodos especificados.
	while( (data_pos != data_upto) | (pointer_pos != pointer_upto) | (DELETE_MODE == 1) ){  // | ((data_pos == 0) & (pointer_pos == 0)) ){
		if ((data_pos < 0) | (pointer_pos < 0)) break;

		// localiza el puntero de datos a borrar.
		node_pointer_to_delete = file_data->block_indirect[pointer_pos];
		aux = (pointerSACBlock*) &(header_start[node_pointer_to_delete]);

		// Indica hasta que nodo debe borrar.
		if (pointer_pos == pointer_upto){
			delete_upto = data_upto;
		} else {
			delete_upto = 0;
		}

		// Borra los nodos de datos que sean necesarios.
		while ((data_pos + DELETE_MODE) != delete_upto){
			node_to_delete = aux[data_pos];
			bitarray_clean_bit(bitarray, node_to_delete);
			bitmap_free_blocks++;
			data_pos--;
		}

		// Si es necesario, borra el nodo de punteros.
		if ((pointer_pos + DELETE_MODE) != pointer_upto){
			bitarray_clean_bit(bitarray, node_pointer_to_delete);
			file_data->block_indirect[pointer_pos] = 0;
			pointer_pos--;
			data_pos = 1023;
		}
	}

	// Cierra el bitmap
	bitarray_destroy(bitarray);
	return 0;
}

/*
 * 	@DESC
 * 		Setea la posicion del pointer_block y un data_block respecto a un archivo
 * 		Se utiliza para localizar donde escribir.
 *
 * 	@PARAM
 * 		pointer_block - puntero al bloque de punteros a settear
 * 		data_block - puntero al bloque de datos a settear
 * 		size - tamanio a escribir del archivo (no se usa, se incorpora pensando en debugging)
 * 		offset - corrimiento que se debe tener del principio del archivo
 *
 * 	@RET
 * 		Devuelve 0. No se contempla error.
 */
int set_position (int *pointer_block, int *data_block, size_t size, off_t offset){
	div_t divi;
	divi = div(offset, (BLOCK_SIZE*PTR_BLOCK_SIZE));
	*pointer_block = divi.quot;
	*data_block = divi.rem / BLOCK_SIZE;
	return 0;
}

int bitarray_test_and_set(t_bitarray *self, off_t offset){
	uint64_t i, res;
	uint64_t *array = (uint64_t*) self->bitarray;

	off_t _off = offset / 64;
	size_t _max = (64);

	for (i=_off; i < (_max); i++){
		if (((array[i]) ^ (~((uint64_t) 0))) != 0){
			break;
		}
	}

	res = i*64;

	for (i=0; i<_max ; i++,res++){
		if (bitarray_test_bit(self, res) == 0){
			bitarray_set_bit(self,res);
			return res;
		}
	}

	return -ENOSPC;
}

/*
 *  @DESC
 *  	Obtiene un bloque libre, actualiza el bitmap.
 *
 *  @PARAM
*		(void)
 *
 *  @RETURN
 *  	Devuelve el numero de un bloque listo para escribir. Si hay error, un numero negativo, correspondiente al error.
 */
int get_node(void){
	t_bitarray *bitarray;
	int res;

	bitarray = bitarray_create((char*) bitmap_start, BITMAP_SIZE_B);

	// Que funcion de la COMMONS es la adyancente a esta???
	res = bitarray_test_and_set(bitarray, SAC_FILE_BY_BLOCK+BITMAP_BLOCK_SIZE+SAC_FILE_BY_TABLE);

	// Cierra el bitmap
	bitarray_destroy(bitarray);
	return res;
}


/*
 * 	@DESC
 * 		Obtiene y registra la cantidad de bloques de datos libres.
 */
int obtain_free_blocks(void){
	t_bitarray *bitarray;
	int free_nodes=0, i;
	int bitmap_size_in_bits = BITMAP_SIZE_BITS;

	bitarray = bitarray_create((char*) bitmap_start, BITMAP_SIZE_B);

	for (i = 0; i < bitmap_size_in_bits; i++){
		if (bitarray_test_bit(bitarray, i) == 0) free_nodes++;
	}

	bitarray_destroy(bitarray);

	bitmap_free_blocks = free_nodes;

	return free_nodes;
}



/*
 *  @DESC
 *  	Actualiza la informacion del archivo.
 *
 *  @PARAM
 *		file_data - El puntero al nodo en el que se encuentra el archivo.
 *		node_number - El numero de nodo que se le debe agregar.
 *
 *  @RET
 *  	Devuelve 0 si salio bien, negativo si hubo problemas.
 */
int add_node(struct sac_file_t *file_data, int node_number){
	int node_pointer_number, position;
	size_t tam = file_data->file_size;
	int new_pointer_block;
	pointerSACBlock *nodo_punteros;

	// Ubica el ultimo nodo escrito y se posiciona en el mismo.
	set_position(&node_pointer_number, &position, 0, tam);

	if((node_pointer_number == BLOCK_INDIRECT-1) & (position == PTR_BLOCK_SIZE-1)) return -ENOSPC;

	// Si es el primer nodo del archivo y esta escrito, debe escribir el segundo.
	// Se sabe que el primer nodo del archivo esta escrito siempre que el primer puntero a bloque punteros del nodo sea distinto de 0 (file_data->blk_indirect[0] != 0)
	// ya que se le otorga esa marca (=0) al escribir el archivo, para indicar que es un archivo nuevo.
	if ((file_data->block_indirect[node_pointer_number] != 0)){
		if (position == 1024) {
			position = 0;
			node_pointer_number++;
		}
	}
	// Si es el ultimo nodo en el bloque de punteros, pasa al siguiente
	if (position == 0){
		new_pointer_block = get_node(); // --------->>>>>>>>> REVISAR ESTA FUNCION!!!!!!!!
		if(new_pointer_block < 0) return new_pointer_block; /* Si sucede que sea menor a 0, contendra el codigo de error */
		memset((char*)&(header_start[new_pointer_block]), 0, BLOCK_SIZE);
		file_data->block_indirect[node_pointer_number] = new_pointer_block;
		// Cuando crea un bloque, settea al siguente como 0, dejando una marca.
		file_data->block_indirect[node_pointer_number +1] = 0;
	} else {
		new_pointer_block = file_data->block_indirect[node_pointer_number]; //Se usa como auxiliar para encontrar el numero del bloque de punteros
	}

	// Ubica el nodo de punteros, relativo al bloque de datos.
	nodo_punteros = (pointerSACBlock*) &data_block_start[new_pointer_block - (SAC_HEADER_BLOCKS + NODE_TABLE_SIZE + BITMAP_BLOCK_SIZE)];

	// Hace que dicho puntero, en la posicion ya obtenida, apunte al nodo indicado.
	nodo_punteros[position] = node_number;

	return 0;

}


/*
 *	@DESC
 *		Obtiene espacio nuevo para un archivo, agregandole los nodos que sean necesarios.
 *		Actualiza el FileSize al tamanio correspondiente.
 *
 *	@PARAM
 *		file_data - El puntero al nodo donde se encuentra el archivo.
 *		size - El tamanio que se le debe agregar.
 *
 *	@RET
 *		0 - Se consiguio el espacio requerido
 *		negativo - Error.
 */
int get_new_space (struct sac_file_t *file_data, int size){
	size_t file_size = file_data->file_size;
	int space_in_block = file_size % BLOCK_SIZE;
	int new_node;
	space_in_block = BLOCK_SIZE - space_in_block; // Calcula cuanto tamanio le queda para ocupar en el bloque

	// Si no hay suficiente espacio, retorna error.
	if ((bitmap_free_blocks*BLOCK_SIZE) < (size - space_in_block)) return -ENOSPC;

	// Actualiza el file size al tamanio que le corresponde:
	if (space_in_block >= size){
		file_data->file_size += size;
		return 0;
	} else {
		file_data->file_size += space_in_block;
	}

	while ( (space_in_block <= size) ){ // Siempre que lo que haya que escribir sea mas grande que el espacio que quedaba en el bloque
		new_node = get_node();
		add_node(file_data, new_node);
		size -= BLOCK_SIZE;
		file_data->file_size += BLOCK_SIZE;
	}

	file_data->file_size += size;

	return 0;
}