#include "muse.h"

void muse_server_init();

int main(void) {

	int response = muse_logger_create();
	if (response < 0)
		return response;

	response = muse_config_load();
	if (response < 0) {
		muse_logger_destroy();
		return response;
	}
	muse_init();
	muse_server_init();
	muse_config_free();
	muse_logger_destroy();
	return EXIT_SUCCESS;
}

void muse_server_init() {
	muse_socket = socket_create_listener("127.0.0.1", muse_get_listen_port());
	if (muse_socket < 0) {
		muse_logger_error("Error al crear server");
		return;
	}

	muse_logger_info("Esperando conexion del libmuse");

	int libmuse_fd = socket_accept_conection(muse_socket);

	if (libmuse_fd == -1) {
		muse_logger_error("Error al establecer conexion con el libmuse");
		return;
	}

	muse_logger_info("Conexion establecida con libmuse");
	int received_bytes;
	int protocol;
	while (1) {
		received_bytes = recv(libmuse_fd, &protocol, sizeof(int), 0);

		if (received_bytes <= 0) {
			muse_logger_error("Error al recibir la operacion del libmuse");
			muse_logger_error("Se desconecto libmuse");
			exit(1);
		}
		switch (protocol) {
		case HANDSHAKE: {
			muse_logger_info("Hi");
			break;
		}
		case MALLOC: {
			muse_logger_info("Malloc received\n");
			t_malloc *malloc_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);
			muse_logger_info("Size to be allocated: %d", malloc_receive->tam);
			muse_logger_info("id proceso %d", malloc_receive->id_libmuse);

			//toda logica para crear un nuevo proceso y segmentacion/paginacion
			t_nodo_proceso* nodoProceso = procesar_id(
					malloc_receive->id_libmuse);
			t_nodo_segmento* nodo_Segmento = crear_nodo_segmento();
			t_malloc_ok* res = malloc(sizeof(t_malloc_ok));
			res->ptr = asignar_dir_memoria(nodo_Segmento, malloc_receive->tam);
			list_add(nodoProceso->list_segmento, nodo_Segmento);

			t_protocol malloc_protocol = MALLOC_OK;
			utils_serialize_and_send(libmuse_fd, malloc_protocol, res);
			break;
		}
		case MEMFREE: {
			muse_logger_info("Free received\n");
			t_free *free_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);

			t_nodo_proceso* nodoProceso = procesar_id(free_receive->self_id);
			int paginaBuscada = free_receive->dir / muse_page_size();

			int dir = free_receive->dir;
			if (estaOcupada(paginaBuscada))
			{
				cambiar_estado_pagina(paginaBuscada, false);

			}

			// Devolver error
			else return;

			muse_logger_info("Direction %d will be freed", free_receive->dir);
			muse_logger_info("id proceso %d", free_receive->self_id);

			//t_nodo_segmento* nodoSegmento=procesar_segmentacion(nodoProceso->list_segmento);

			t_free_response* free_res = malloc(sizeof(t_free_response));
			free_res->res = 1;
			t_protocol free_protocol = FREE_RESPONSE;
			utils_serialize_and_send(libmuse_fd, free_protocol, free_res);
			break;
		}

		case GET: {
			muse_logger_info("Get received");
			t_get *get_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);
			muse_logger_info(
					"%d bytes will be transfered starting at direction: %d",
					get_receive->size, get_receive->src);
			muse_logger_info("id proceso %d", get_receive->id_libmuse);
			t_nodo_proceso* nodo = procesar_id(get_receive->id_libmuse);

			// VErifico que haya contenido en la posicion
			char* content = malloc(sizeof(get_receive->size));
			memcpy(content, memoria + get_receive->src, sizeof(content));

			if (strlen(content) != 0)
			{
				// Response logic
				t_get_ok* response = malloc(sizeof(t_get_ok));
				response->res = content;
				response->tamres = strlen(content)* sizeof(char);
				t_protocol get_protocol = GET_OK;
				utils_serialize_and_send(libmuse_fd, get_protocol, response);
			}
			// Else
			break;
		}

		case COPY: {
			muse_logger_info("Copy received");
			t_copy* cpy = utils_receive_and_deserialize(libmuse_fd, protocol);
			muse_logger_info(
					"Process with pid; %d is trying to copy %d bytes to direction: %d",
					cpy->self_id, cpy->size, cpy->dst);
			muse_logger_info("id proceso %d", cpy->self_id);
			t_nodo_proceso* nodo = procesar_id(cpy->self_id);

			// Verifico si puedo copiar algo a la posicion solicitada

			t_heapMetadata *heap = malloc(sizeof(t_heapMetadata))
			memcpy(heap, memoria + cpy->dst -5, sizeof(t_heapMetadata));

			if (heap->libre)
			{
				int pagina = cpy->dst / muse_page_size() +1;
				heap->libre = false;
				heap->size = cpy->size;
				memcpy (cpy->dst-5, heap, sizeof(t_heapMetadata));
				memcpy (cpy->dst, cpy->content, cpy->size);

			}

			// Response logic
			t_copy_response* copy_res = malloc(sizeof(t_copy_response));
			copy_res->res = 1;
			t_protocol cpy_protocol = GET_OK;
			utils_serialize_and_send(libmuse_fd, cpy_protocol, copy_res);
			break;
		}
		case SYNC: {
			// TODO: Implementation
		}
		case MAP: {
			// TODO: Implementation
		}
		case UNMAP: {
			// TODO: Implementation
		}
		}
	}

}
void muse_init() {
	memoria = malloc(muse_memory_size());
	int cantidad_paginas_reales = muse_memory_size() / muse_page_size();
	int cantidad_paginas_virtuales = muse_swap_size() / muse_page_size();
	cantidad_paginas_totales = cantidad_paginas_reales
			+ cantidad_paginas_virtuales;
   /*
	t_vector_paginas creacionVectorPaginas[cantidad_paginas_totales];

	for (int i = 0; i < cantidad_paginas_totales; i++) {
		creacionVectorPaginas[i].libre = true;
	}
	vectorPaginas = creacionVectorPaginas;
	*/
	vectorPaginas = (t_vector_paginas*)calloc(cantidad_paginas_totales, sizeof(vectorPaginas));
	for (int i = 0; i < cantidad_paginas_totales; i++) {
		vectorPaginas[i].libre = true;
	}

	lista_procesos = list_create();
	puts("algo");
	printf("cantidad de bytes de la estructura: %d", sizeof(t_heapMetadata));
	printf("\ncantidad de paginas reales: %d", cantidad_paginas_reales);
	printf("\ncantidad de paginas virtuales: %d", cantidad_paginas_virtuales);
	printf("\ncantidad de paginas totales: %d\n", cantidad_paginas_totales);
}

t_nodo_proceso *crear_nodo(int id) {
	t_nodo_proceso* nodo = malloc(sizeof(t_nodo_proceso));
	nodo->id = id;
	nodo->list_segmento = list_create();
	return nodo;
}

t_nodo_proceso* existe_proceso_en_lista(int id) {
	int existeNodoEnLaLista(t_nodo_proceso *nodo_proceso) {
		printf("id recibido: %d \n", id);
		printf("id en nodo: %d \n", nodo_proceso->id);
		return nodo_proceso->id == id;
	}
	return list_find(lista_procesos, (void*) existeNodoEnLaLista);
}

t_nodo_proceso* procesar_id(int id) {
	t_nodo_proceso* unNodo = existe_proceso_en_lista(id);
	if (unNodo == NULL) {
		muse_logger_info("Creacion de nodo proceso");
		unNodo = crear_nodo(id);
		list_add(lista_procesos, unNodo);

	}
	return unNodo;
}
int asignarPaginaLibre() {
	int paginaLibre = -1;
	for (int i = 0; i < cantidad_paginas_totales; i++) {
		t_vector_paginas pagina = *(vectorPaginas + i);
		if (pagina.libre) {
			paginaLibre = i;
			break;
		}
	}
	return paginaLibre;

}

t_nodo_segmento* crear_nodo_segmento() {
	t_nodo_segmento* nodoSegmento = malloc(sizeof(t_nodo_segmento));
	//i
	//printf("pagina libre %d",paginaLibre );
	nodoSegmento->base = 0;
	nodoSegmento->tamanio = 0;
	nodoSegmento->list_paginas = list_create();
	return nodoSegmento;
}

int asignar_dir_memoria(t_nodo_segmento* nodoSegmento,
		uint32_t memoria_reservar) {
	int memoria_reservar2  = memoria_reservar +2;

	int pagina = -1;
	if (list_size(nodoSegmento->list_paginas) == 0) {
		int cantidad_paginas_necesarias = (memoria_reservar2 / muse_page_size())
				+ 1;
		int primera_pagina;
		if (existe_memoria_parar_paginas(cantidad_paginas_necesarias)) {
			for (int i = 0; i < cantidad_paginas_necesarias; i++) {
				int paginaLibre = asignarPaginaLibre();
				if (i == 0) {
					primera_pagina = paginaLibre;
				}
				cambiar_estado_pagina(paginaLibre, false);
				t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
				nodo_pagina->nro = paginaLibre;
				list_add(nodoSegmento->list_paginas, nodo_pagina);
			}
			nodoSegmento->base    = primera_pagina * muse_page_size();
			nodoSegmento->tamanio = cantidad_paginas_necesarias * muse_page_size() -1;
			t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
			heap->libre = false;
			heap->size = memoria_reservar;
			memcpy(memoria +primera_pagina * muse_page_size(), heap,
					sizeof(t_heapMetadata));
			heap->libre = true;
			heap->size  = cantidad_paginas_necesarias * muse_page_size() - memoria_reservar -5 -5 -1;
			memcpy(memoria +5+primera_pagina* muse_page_size() + memoria_reservar,
					heap, sizeof(t_heapMetadata));
			pagina = primera_pagina;
			memoria_reservar = 5;
			recorer_segmento_espacio_libre(nodoSegmento,memoria_reservar);

		} else {
			return -1;
		}

	}
//falta logica else
//buscar en segmento espacio libre
//buscar si segmento se puede agrandar
//crear nuevo segmento
	return pagina * muse_page_size()+5;
}

int recorer_segmento_espacio_libre(t_nodo_segmento* nodoSegmento,uint32_t memoria_reservar){
	int desde = nodoSegmento->base;
	int hasta= nodoSegmento->base + nodoSegmento->tamanio;

	int primera_pagina = (desde / muse_page_size());

	int ultima_pagina = (hasta/ muse_page_size())+1;

	int offset = primera_pagina*muse_page_size();
	int respuesta = -1;
	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
	printf("\n base segmento %d \n",nodoSegmento->base);
	printf("limite segmento %d \n",ultima_pagina*muse_page_size()-1);
	do{
		memcpy(heap,memoria+offset,sizeof(t_heapMetadata));
		offset =offset + 5;
		printf(" \n offset despues de leer estructura %d \n",offset);
		if(heap->libre && memoria_reservar < heap->size){
			printf("\ **** esta libre a partir de este lugar %d ***** \n",offset);
			respuesta = offset;
			break;

		}

		if(heap->libre && heap->size  <= memoria_reservar){
			respuesta = offset;
			break;
		}
		printf(" \n memoria reservar %d \n",memoria_reservar);
		printf(" \n heap size de la estructura %d \n",heap->size);

		offset = offset + heap->size;
		printf("offset despues de leer el size de  la estructura %d \n",offset);
		//sobran menos de 5 bytes
		if(ultima_pagina*muse_page_size()-1 -offset  <= 4 ){
			break;
		}
	}while (offset <= ultima_pagina*muse_page_size()-1);

	printf("valor respuesta: %d \n",respuesta);
	return respuesta;
}

bool existe_memoria_parar_paginas(int cantidad_paginas_necesarias) {
	int i = 0;
	int contador =0;
	while (i < cantidad_paginas_totales) {

		if (vectorPaginas[i].libre) {
			contador++;
		}
		if (contador == cantidad_paginas_necesarias) {
			break;
		}
		i++;
	}
	if(contador == cantidad_paginas_necesarias){
		return true;
	}
	return false;
}

void cambiar_estado_pagina(int pagina, bool estado) {
	vectorPaginas[pagina].libre = estado;
}

bool estaOcupada(int pagina)
{
	return vectorPaginas[pagina].libre;
}

/*
 t_nodo_segmento* nuevo_segmento(uint32_t cantidad_memoria){
 int cantidad_paginas_necesarias= cantidad_memoria /muse_page_size();
 if(existeCantidadPaginasNecesarias(cantidad_paginas_necesarias)){

 }
 else{
 return null;
 }
 }
 */

