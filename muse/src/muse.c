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
	printf("entra al init");
	muse_init();
	muse_server_init();
	muse_config_free();
	muse_logger_destroy();
	return EXIT_SUCCESS;
}

void muse_server_init() {
	indice = 0;
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
			t_malloc_ok* res = malloc(sizeof(t_malloc_ok));
			int cantidad_segmentos = list_size(nodoProceso->list_segmento);
			if (cantidad_segmentos == 0) {
				t_nodo_segmento* nodo_Segmento = crear_nodo_segmento();
				nodo_Segmento->tipo = S_ALLOC;
				nodo_Segmento->map = NULL;
				res->ptr = asignar_dir_memoria(nodo_Segmento,
						malloc_receive->tam);
				//estado_heap(nodo_Segmento);
				list_add(nodoProceso->list_segmento, nodo_Segmento);
			} else {
				t_nodo_segmento* nodoSegmento;
				int i;
				int resp;
				for (i = 0; i < cantidad_segmentos; i++) {
					nodoSegmento = list_get(nodoProceso->list_segmento, i);
					//estado_heap(nodoSegmento);
					resp = -1;
					if (nodoSegmento->tipo == S_ALLOC) {
						muse_logger_info("Recorrer Segmento");
						resp = recorrer_segmento_espacio_libre(nodoSegmento,
								malloc_receive->tam);

					}
					if (resp != -1) {
						break;
					}
					printf(" \n direccion virtual asignada %d \n",resp);
				}
				//agrando el ultimo alloc si no se pudo agrandar buscar un espacio libre
				if (resp == -1) {
					//muse_logger_info("Agrandar Segmento");
					resp = agrandar_segmento(nodoSegmento, malloc_receive->tam);
					//muse_logger_info(" \n direccion virtual asignada %d \n",resp);
				}
				res->ptr = resp;

			}

			t_protocol malloc_protocol = MALLOC_OK;
			utils_serialize_and_send(libmuse_fd, malloc_protocol, res);
			break;
		}
		case MEMFREE: {
			muse_logger_info("Free received\n");
			t_free *free_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);
			muse_logger_info("Direction %d will be freed \n",
					free_receive->dir);

			if (!existe_proceso_en_lista(free_receive->self_id)) {
				t_protocol free_failed = SEG_FAULT;
				send(libmuse_fd, &free_failed, sizeof(t_protocol), 0);
				break;
			}

			int dir = free_receive->dir;
			t_heapMetadata* header = malloc(sizeof(t_heapMetadata));

			memcpy(header, memoria + dir - 5, sizeof(header));

			if (header->libre) {
				t_protocol free_failed = SEG_FAULT;
				send(libmuse_fd, &free_failed, sizeof(t_protocol), 0);
				break;
			}

			else {
				header->libre = true;
				memset(memoria + dir + 5, '\0', header->size);
				t_heapMetadata* closestHeader = malloc(sizeof(t_heapMetadata));
				memcpy(closestHeader, memoria + dir + header->size,
						sizeof(closestHeader));
				int oldsize = header->size;

				if (closestHeader->libre) {
					header->size += closestHeader->size;
					memset(memoria + dir + header->size, '\0', header->size);
				}

				t_heapMetadata* previousHeaderDir = malloc(
						sizeof(previousHeaderDir));
				int desp = 0;

				do {
					memcpy(previousHeaderDir, memoria + desp,
							sizeof(previousHeaderDir));
					if (memoria + desp + previousHeaderDir->size == dir - 6) {
						if (previousHeaderDir->libre) {
							previousHeaderDir->size += header->size;
							memset(memoria + desp + 5, '\0',
									previousHeaderDir->size);
							break;
						}
					} else
						desp = desp + previousHeaderDir->size + 1;
				}

				while (desp < dir - 5);
			}

			muse_logger_info("Direction %d will be freed", free_receive->dir);

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

				t_nodo_segmento *nodoSegmento = buscar_segmento(
						get_receive->src, nodo->list_segmento);

				t_protocol copy_protocol = GET_OK;
				if (nodoSegmento == NULL) {
					copy_protocol = SEG_FAULT;
					send(libmuse_fd, &copy_protocol, sizeof(protocol), 0);
					break;
				}
				if ((nodoSegmento->base + nodoSegmento->tamanio)
						< get_receive->src + get_receive->size) {
					copy_protocol = SEG_FAULT;
					send(libmuse_fd, &copy_protocol, sizeof(protocol), 0);
					break;
				}
				if(nodoSegmento->tipo == S_ALLOC){
					t_heapMetadata* heap = obtener_heap(get_receive->src ,nodoSegmento);

								if (!heap->libre && heap->size >= get_receive->size) {
									get_contenido_virtual (get_receive->src,nodoSegmento ,get_receive->size);
									t_copy* copy_send = malloc(sizeof(t_copy));
									copy_send->self_id = get_receive->id_libmuse;
									copy_send->size = get_receive->size;
									copy_send->dst = get_receive->src;
									copy_send->content = malloc(get_receive->size);
									copy_send->content = get_contenido_virtual (get_receive->src, nodoSegmento ,get_receive->size);
									//memcpy(copy_send->content, content, get_receive->size);
									utils_serialize_and_send(libmuse_fd, copy_protocol,
											copy_send);

									}

							else {
								copy_protocol = SEG_FAULT;
								send(libmuse_fd, &copy_protocol, sizeof(protocol), 0);
							}

				}
				else{
					get_contenido_virtual (get_receive->src,nodoSegmento ,get_receive->size);
					t_copy* copy_send = malloc(sizeof(t_copy));
				    copy_send->self_id = get_receive->id_libmuse;
					copy_send->size = get_receive->size;
					copy_send->dst = get_receive->src;
					copy_send->content = malloc(get_receive->size);
					copy_send->content = get_contenido_virtual (get_receive->src, nodoSegmento ,get_receive->size);
					utils_serialize_and_send(libmuse_fd, copy_protocol,
																copy_send);

				}

				break;
			//}
		}

		case COPY: {
			muse_logger_info("Copy received");
			t_copy* cpy = utils_receive_and_deserialize(libmuse_fd, protocol);

			printf("%d content \n", *((int*) cpy->content));

			muse_logger_info(
					"Process with pid; %d is trying to copy %d bytes to direction: %d",
					cpy->self_id, cpy->size, cpy->dst);

			t_nodo_proceso* nodoProceso = procesar_id(cpy->self_id);
			t_nodo_segmento *nodoSegmento = buscar_segmento(cpy->dst,
					nodoProceso->list_segmento);
			printf("\n ****--base %d", nodoSegmento->base);
			printf("\n ****--tamanio %d", nodoSegmento->tamanio);
			t_protocol cpy_protocol;
			if (!existe_proceso_en_lista(cpy->self_id)) {
				t_copy_response* copy_res = malloc(sizeof(t_copy_response));
				copy_res->res = -1;
				cpy_protocol = SEG_FAULT;
				utils_serialize_and_send(libmuse_fd, cpy_protocol, copy_res);
				break;
			} else {
				copy_contenido_virtual (cpy->dst , nodoSegmento , cpy->size,cpy->content);
				t_protocol cpy_protocol = GET_OK;
				send(libmuse_fd, &cpy_protocol, sizeof(protocol), 0);
			}
			break;
		}
		case SYNC: {
			muse_logger_info("SYNC received\n");
			t_msync *msync_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);
			muse_logger_info("id proceso %d", msync_receive->id_libmuse);
			muse_logger_info("size %d",msync_receive->size );
			muse_logger_info("src %d",msync_receive->src );


			t_nodo_proceso* nodo = procesar_id(msync_receive->id_libmuse);

			t_nodo_segmento *nodoSegmento;
			for(int i=0;i<list_size(nodo->list_segmento);i++){
				nodoSegmento = list_get(nodo->list_segmento,i);
				if(nodoSegmento->base < msync_receive->src){
					if(nodoSegmento->base+nodoSegmento->tamanio > msync_receive->src){
						sincronizar(nodoSegmento, msync_receive->size);
						send(libmuse_fd, &protocol, sizeof(protocol), 0);
						break;
					}
				}
			}

			int size = list_size(nodo->list_segmento);
			printf("\n size %d",size);


			printf("\n Segmento base %d", nodoSegmento->base);
			printf("\n Segmento tamanio %d",nodoSegmento->tamanio);

			break;
		}
		case MAP: {
			// TODO: Implementation
			muse_logger_info("MAP received\n");
			t_mmap *map_receive = utils_receive_and_deserialize(libmuse_fd,
					protocol);
			muse_logger_info("**** path %s*****", map_receive->path);
			muse_logger_info("****id proceso %d*****", map_receive->id_libmuse);
			int flag;
			t_nodo_segmento* nodo_Segmento = crear_nodo_segmento();
			nodo_Segmento->map = NULL;
			if (map_receive->flag == MAP_PRIVATE) {
				muse_logger_info("**** MAP privado *****");
				nodo_Segmento->tipo = MAP_PRIVATE;
				tipo_map_private *type = malloc(sizeof(tipo_map_private));
				type->path = strdup(map_receive->path);
				nodo_Segmento->map=type;
			}
			if (map_receive->flag == MAP_SHARED) {
				muse_logger_info("**** MAP compartido *****");
				nodo_Segmento->tipo = MAP_SHARED;
				tipo_map_shared *type = malloc(sizeof(tipo_map_shared));
				type->cant_links = 1;
				type->path = strdup(map_receive->path);
				nodo_Segmento->map = type;
			}

			t_nodo_proceso* nodoProceso = procesar_id(map_receive->id_libmuse);

			int cantidad_segmentos = list_size(nodoProceso->list_segmento);

			int cantidad_paginas = (map_receive->size / muse_page_size() + 1);
			int tamanio = cantidad_paginas * muse_page_size();

			muse_logger_info("****cantidad de paginas para map %d*****",
					cantidad_paginas);

			nodo_Segmento->tamanio = tamanio;
			if (cantidad_segmentos == 0) {
				nodo_Segmento->base = 0;
				list_add(nodoProceso->list_segmento, nodo_Segmento);
			} else {
				t_nodo_segmento* otroNodoSegmento = list_get(
						nodoProceso->list_segmento, cantidad_segmentos - 1);
				nodo_Segmento->base = otroNodoSegmento->base
						+ otroNodoSegmento->tamanio + 1;
			}
			nodo_Segmento->list_paginas = list_create();
			for (int i = 0; i < cantidad_paginas; i++) {
				t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
				nodo_pagina->indiceVector = asignarIndiceVectorLibre();
				list_add(nodo_Segmento->list_paginas, nodo_pagina);
				//no necesito un frame
				crear_nodo_indice_algoritmo(nodo_pagina->indiceVector, -1, 0);
			}
			t_malloc_ok* map_res = malloc(sizeof(t_malloc_ok));
			map_res->ptr = nodo_Segmento->base;

			t_protocol cpy_protocol = MAP_OK;
			utils_serialize_and_send(libmuse_fd, cpy_protocol, map_res);

			break;

		}
		case UNMAP: {
			// TODO: Implementation
		}
		}
	}

}
void muse_init() {
	crear_archivo_swap ();
	flag_algoritmo = 0;
	lista_algoritmo = list_create();
	memoria = malloc(muse_memory_size());
	memset(memoria,0,muse_memory_size());
	int cantidad_paginas_reales = muse_memory_size() / muse_page_size();
	printf(" \n cantidad de frames %d",cantidad_paginas_reales);
	int diferencia = cantidad_frames % 8;
	path_bitmap = "bitmap.bit";
	cantidad_frames = cantidad_paginas_reales;
	creacionDeArchivoBitmap(cantidad_frames, path_bitmap);

	bitarray = crearBitmap(cantidad_frames, diferencia, path_bitmap);
	int cantidad_paginas_virtuales = muse_swap_size() / muse_page_size();
	cantidad_paginas_totales = cantidad_paginas_reales
			+ cantidad_paginas_virtuales;

	int cantidad_paginas_swap = muse_swap_size() / muse_page_size();
	diferencia = cantidad_paginas_swap % 8;
	path_bitmap = "bitmap_swap.bit";
	cantidad_frames_swap = cantidad_paginas_swap;
	creacionDeArchivoBitmap(cantidad_frames_swap, path_bitmap);

	bitarray_swap = crearBitmap(cantidad_frames, diferencia, path_bitmap);

	vectorFrames = (t_vector_frames*) calloc(cantidad_paginas_totales,
			sizeof(t_vector_frames));
	vectorAtributoPaginas = (t_vector_atributo_paginas*) calloc(
			cantidad_paginas_totales, sizeof(t_vector_atributo_paginas));

	for (int i = 0; i < cantidad_paginas_totales; i++) {
		vectorFrames[i].libre = true;
	}

	for (int i = 0; i < cantidad_paginas_totales; i++) {
		vectorAtributoPaginas[i].libre = true;
		vectorAtributoPaginas[i].presencia = true;
		vectorAtributoPaginas[i].frame = -1;
		vectorAtributoPaginas[i].modificado = 0;
		vectorAtributoPaginas[i].uso = 0;
	}

	lista_procesos = list_create();
	//printf("cantidad de bytes de la estructura: %d", sizeof(t_heapMetadata));
	//printf("\ncantidad de paginas reales: %d", cantidad_paginas_reales);
	printf("\ncantidad de paginas virtuales: %d", cantidad_frames);
	printf("\ncantidad de paginas totales: %d\n", cantidad_frames_swap);

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

int asignarFrameLibre() {
	int libre = frameLibre();
	if (libre == -1) {
		printf("\n aplicar algoritmo \n");
		libre = aplicar_algoritmo();
		muse_logger_info("Libero frame %d por algoritmo ",libre);
	}
	return libre;

}

int asignarFrameLibreSWAP() {
	return frameLibreSWAP();
}

int asignarIndiceVectorLibre() {
	indice++;
	return indice - 1;
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
	int memoria_reservar2 = memoria_reservar + 10;
	int cantidad_paginas_necesarias;
	int pagina = -1;
	if (list_size(nodoSegmento->list_paginas) == 0) {
		cantidad_paginas_necesarias = (memoria_reservar2 / muse_page_size())
				+ 1;
		int primerFrameLibre;
		int frameLibre;
		if (existe_memoria_para_frames(cantidad_paginas_necesarias)) {
			pagina = 0;
			for (int i = 0; i < cantidad_paginas_necesarias; i++) {

				frameLibre = asignarFrameLibre();
				int indiceLibre = asignarIndiceVectorLibre();

				printf("frame libre: %d \n", frameLibre);
				printf("indice libre para pagina: %d \n", indiceLibre);

				//cambio a ocupado
				cambiar_estado_frame(frameLibre, true);

				t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
				nodo_pagina->indiceVector = indiceLibre;

				//falta c
				crear_nodo_indice_algoritmo(indiceLibre, frameLibre, 1);

				list_add(nodoSegmento->list_paginas, nodo_pagina);
				if (i == 0) {
					primerFrameLibre = frameLibre;
				}

			}

			//ESto se va a usar para calcular donde escribir la estructura
			//ahora hace escritura contigua
			t_list *lista_frames_proceso = list_create();
			for (int i = 0; i < list_size(nodoSegmento->list_paginas); i++) {
				t_nodo_pagina* nodoPagina = list_get(nodoSegmento->list_paginas,
						i);
				t_frames* frame = malloc(sizeof(t_frames));
				printf("\n indice vector: %d ", nodoPagina->indiceVector);
				printf("\n size lista algoritmo: %d ",
						list_size(lista_algoritmo));
				t_nodo_atributo_paginas * nodoAlgoritmo = nodo_algoritmo(
						nodoPagina->indiceVector);
				frame->nroFrame = nodoAlgoritmo->frame;
				list_add(lista_frames_proceso, frame);
			}

			nodoSegmento->tamanio = cantidad_paginas_necesarias
					* muse_page_size() - 1;

			t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
			heap->libre = false;
			heap->size = memoria_reservar;

			memcpy(memoria + primerFrameLibre * muse_page_size(), heap,
					sizeof(t_heapMetadata));

			int bytes_sobran = cantidad_paginas_necesarias * muse_page_size()
					- memoria_reservar - 5;
			int dir_proxima_estructura = frameLibre * muse_page_size()
					+ (muse_page_size() - bytes_sobran);

			heap->libre = true;
			heap->size = bytes_sobran - 5 - 1;

			memcpy(memoria + dir_proxima_estructura, heap,
					sizeof(t_heapMetadata));
			//test_swap();
			printf("cantidad de paginas %d \n", cantidad_paginas_necesarias);
			printf("tamanio de cantidad de paginas %d \n",
					cantidad_paginas_necesarias * muse_page_size());
			printf("contenido de base segmento %d \n", nodoSegmento->base);
			printf("bytes sobran con la estructura %d \n", bytes_sobran);
			printf("bytes sobran sin la estructura %d \n", bytes_sobran - 5);
			printf("memoria a reservar %d \n", memoria_reservar);
			printf("ultimo frame %d \n", frameLibre);
			printf("ubicacion logica siguiente estructura %d \n",
					nodoSegmento->base + memoria_reservar + 5);
			printf("ubicacion fisica siguiente estructura %d \n",
					dir_proxima_estructura);
			printf("cantidad de bytes q sobran %d \n", bytes_sobran);

		} else {
			return -1;
		}

	}
//falta logica else
//buscar en segmento espacio libre
//buscar si segmento se puede agrandar
//crear nuevo segmento
	return pagina * muse_page_size() + 5;
}

int recorrer_segmento_espacio_libre(t_nodo_segmento* nodoSegmento,
		uint32_t memoria_reservar) {
	int desde = nodoSegmento->base;
	int hasta = nodoSegmento->base + nodoSegmento->tamanio;
	int primera_pagina = (desde / muse_page_size());
	int ultima_pagina = list_size(nodoSegmento->list_paginas);
	int respuesta = -1;
	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));

	int offset = 0;
	int pagina = 0;
	int dir_virtual = nodoSegmento->base;
	do {
		heap = obtener_heap(dir_virtual , nodoSegmento);
		printf(" \n direccion virtual %d \n", dir_virtual);
		printf(" \n heap size %d \n", heap->size);
		offset = offset + 5;
		if (heap->libre == true) {
			if ((memoria_reservar + 5) < heap->size) {
				respuesta = offset;
				printf("\n **** esta libre a partir de este lugar %d ***** \n",
						offset);
				int size_heap = heap->size;
				t_heapMetadata *nuevo_heap = malloc(sizeof(t_heapMetadata));
				heap->libre = false;
				heap->size = memoria_reservar;
				//printf("dir actual: %d \n", offset);
				offset = offset - 5;
				actualizar_heap(heap,dir_virtual,nodoSegmento);
				offset = offset + 5 + heap->size;
				dir_virtual = offset;
				heap->libre = true;
				heap->size = size_heap - memoria_reservar - 5;
				actualizar_heap(heap,offset,nodoSegmento);
				printf("****proxima direccion virtual libre: %d****** \n", offset);
				break;
			}
		}
		offset = offset + heap->size;
		dir_virtual = offset;
		printf("****offset: %d****** \n", offset);
		printf("****virtual: %d****** \n", offset);

	} while (offset < (nodoSegmento->base + nodoSegmento->tamanio) - 1);

	return respuesta;
}

bool existe_memoria_para_frames(int cantidad_paginas_necesarias) {
	int i = 0;
	int contador = 0;
	while (i < cantidad_paginas_totales) {

		if (bitarray_test_bit(bitarray, i) == 0) {
			contador++;
		}
		if (contador == cantidad_paginas_necesarias) {
			break;
		}
		i++;
	}
	if (contador == cantidad_paginas_necesarias) {
		return true;
	}
	return false;
}

//true   1 como ocupado
//false  0 como libre
void cambiar_estado_frame(int frame, bool estado) {

	printf("-----cambiar frame de MP %d\n", frame);

	if (estado == false) {
		printf("-----cambiar a libre %d\n", frame);
		bitarray_clean_bit(bitarray, frame);

	}
	if (estado == true) {
		printf("-----cambiar a ocupado %d\n", frame);
		bitarray_set_bit(bitarray, frame);
	}
}

void cambiar_estado_frame_swap(int frame, bool estado) {

	printf("\n-----cambiar frame de SWAP %d\n", frame);

	if (estado == false) {
		printf("-----cambiar a libre %d\n", frame);
		bitarray_clean_bit(bitarray_swap, frame);

	}
	if (estado == true) {
		printf("-----cambiar a ocupado %d\n", frame);
		bitarray_set_bit(bitarray_swap, frame);
	}
}

bool estaOcupada(int frame) {
	return bitarray_test_bit(bitarray, frame);
}

void estadoFrames() {
	for (int i = 0; i < cantidad_frames; i++) {
		//printf("\n posicion %d libre",bitarray_test_bit(bitarray,i));

	}
}

int agrandar_segmento(t_nodo_segmento* nodoSegmento, uint32_t memoria_reservar) {
	int desde = nodoSegmento->base;
	int hasta = nodoSegmento->base + nodoSegmento->tamanio;

	int primera_pagina = (desde / muse_page_size());
	int ultima_pagina = list_size(nodoSegmento->list_paginas) - 1;

	int dir_virtual = nodoSegmento->base;
	int dir_fisica = 0;

	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));

	int offset = 0;
	int pagina = primera_pagina;
	int respuesta;
	do {
		heap =obtener_heap(dir_virtual ,nodoSegmento);

		printf("offset antes de leer la estructura %d \n", offset);
		offset = offset + 5;

		printf("offset despues de leer la estructura %d \n", offset);
		offset = offset + heap->size;

		printf("offset despues de leer el size %d \n", offset);

		dir_virtual = offset;

	} while (offset < (nodoSegmento->base + nodoSegmento->tamanio) - 1);

	offset = offset - heap->size - 5;
	respuesta = offset;
	printf(" \n offset de la estructura libre %d \n", offset);
	printf(" \n bytes libres %d \n", heap->size);

	int bytes_libres = heap->size;
	int bytes_faltan = (memoria_reservar + 5) - bytes_libres;

	//agregada paginas al segmento
	int paginas_necesarias = agregar_paginas (bytes_faltan ,nodoSegmento);


	nodoSegmento->tamanio = nodoSegmento->tamanio
			+ paginas_necesarias * muse_page_size();

	int total = heap->size + paginas_necesarias * muse_page_size();

	printf(" \n bytes libres  %d + 32 %d \n", bytes_libres, total);

	heap->size = memoria_reservar;
	heap->libre = false;

	actualizar_heap(heap,offset,nodoSegmento);

	printf(" \n memoria reservar %d \n", memoria_reservar);
	printf(" \n offset %d \n", offset);

	offset = offset + 5 + memoria_reservar;

	heap->size = total - memoria_reservar - 5;
	heap->libre = true;

	actualizar_heap(heap,offset,nodoSegmento);

	printf("\n ******bytes libres %d****** \n", heap->size);
	printf("\n ******offset de estructura libre %d****** \n", offset);

	dir_virtual = offset;

	printf("\n ******dirvirtual %d****** \n", offset);
	printf("\n ******pagina %d****** \n", pagina);


	printf("\n ******respuesta %d****** \n", respuesta);

	return respuesta+5;

}

t_nodo_segmento *buscar_segmento(uint32_t src, t_list *list) {
	int existeNodoEnLaLista(t_nodo_segmento *nodo_proceso) {
		printf("\n base %d", nodo_proceso->base);
		printf("\n tamanio %d", nodo_proceso->tamanio);
		return ((nodo_proceso->base < src)
				&& ((nodo_proceso->base + nodo_proceso->tamanio) > src));
	}
	return list_find(list, (void*) existeNodoEnLaLista);
}

void sincronizar(t_nodo_segmento *segmento, size_t size) {
	int pagina = 0;
	int copiado = size;
	void *contenido_pagina = malloc(muse_page_size());
	do {
		t_nodo_pagina * nodo_pagina = list_get(segmento->list_paginas, pagina);

		t_nodo_atributo_paginas *nodoAlgoritmo = nodo_algoritmo(
				nodo_pagina->indiceVector);
		int frame_libre;
		int frame = nodoAlgoritmo->frame;
		if (nodoAlgoritmo->modificado == 1) {
			if (nodoAlgoritmo->presencia == 0) {
				frame_libre = asignarFrameLibre();
				if (frame_libre < 0) {
					frame_libre = aplicar_algoritmo();
				}
				cambiar_estado_frame(frame_libre, true);
				void * pagina_swap = traer_swap(frame);
				memcpy(memoria + frame_libre * muse_page_size(), pagina_swap,
						muse_page_size());
				frame = frame_libre;
				free(pagina_swap);
			}
			memcpy(contenido_pagina, memoria + frame * muse_page_size(),
					muse_page_size());
			nodoAlgoritmo->presencia = 1;
			nodoAlgoritmo->modificado = 0;
			nodoAlgoritmo->frame = frame;
			char *path = path_segmento(segmento);
			flush(pagina, contenido_pagina, path);
		}
		pagina++;
		copiado = copiado - muse_page_size();
	} while (copiado > 0 && (pagina < list_size(segmento->list_paginas)));
}

char * path_segmento(t_nodo_segmento* segmento) {
	if (segmento->tipo == MAP_SHARED) {
		char * path = ((tipo_map_shared*) segmento->map)->path;
		return path;
	} else {
		char * path = ((tipo_map_private*) segmento->map)->path;
		return path;
	}
}

t_bitarray *crearBitmap(int cantidadDepagina, int diferencia, char * path) {

	//FILE* bloque_crear;
	//bloque_crear->_IO_buf_base
	char *direccionArchivoBitMap = path;
	int bitmap = open(direccionArchivoBitMap, O_RDWR);
	struct stat mystat;
	//puts(bitmap);
	if (fstat(bitmap, &mystat) < 0) {
		//printf("Error al establecer fstat\n");
		close(bitmap);
	}
	char *bmap;
	bmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED,
			bitmap, 0);

	if (bmap == MAP_FAILED) {
		//printf("Error al mapear a memoria: %s\n", strerror(errno));

	}

	size_t tamanio = cantidadDepagina / 8;
	if (diferencia > 0) {
		tamanio++;

	}

	t_bitarray *unbitarray = bitarray_create_with_mode(bmap, tamanio,
			MSB_FIRST);
	size_t cantidadDebits = bitarray_get_max_bit(unbitarray);
	for (int i = 0; i < cantidadDebits; i++) {
		//bitarray_clean_bit(bitarray,i);
		//printf("*****posicion %d valor %d *****:\n",i,bitarray_test_bit(unbitarray,i));

	}
	return unbitarray;

}

t_nodo_atributo_paginas * nodo_algoritmo(int unIndice) {
	int existeNodoEnLaLista(t_nodo_atributo_paginas *nodo_proceso) {
		return nodo_proceso->unIndice == unIndice;
	}
	return list_find(lista_algoritmo, (void*) existeNodoEnLaLista);
}

void crear_nodo_indice_algoritmo(int indice, int frame, int presencia) {
	t_nodo_atributo_paginas* nodo = malloc(sizeof(t_nodo_atributo_paginas));
	nodo->frame = frame;
	nodo->modificado = 0;
	nodo->presencia = presencia;
	nodo->unIndice = indice;
	nodo->uso = 0;
	list_add(lista_algoritmo, nodo);

}

int frameLibre() {
	size_t cantidadDebits = cantidad_frames;
	int i;
	int libre = -1;
	for (i = 0; i < cantidadDebits; i++) {
		if (bitarray_test_bit(bitarray, i) == 0) {
			libre = i;
			printf(" \n se encontro el frame libre : %d",i);
			break;
		}
	}

	return libre;
}

int frameLibreSWAP() {
	size_t cantidadDebits = cantidad_frames_swap;
	int i;
	int libre = -1;
	for (i = 0; i < cantidadDebits; i++) {
		if (bitarray_test_bit(bitarray_swap, i) == 0) {
			libre = i;
			break;
		}
	}
	return libre;
}

void flush(int pagina, char *contenido_pagina, char *path) {
	grabar_archivo(path, muse_page_size(), pagina * muse_page_size(),
			contenido_pagina);
}

int aplicar_algoritmo() {
	int size = list_size(lista_algoritmo);
	int i = flag_algoritmo;
	int flag = 0;
	int frame = -1;
	printf(" \n *****while *****");
	estado_algoritmo();
	while (flag == 0) {
		if(i == size){
			i = 0;
		}
		t_nodo_atributo_paginas * nodo = list_get(lista_algoritmo, i);

		printf(" \n nodo %d",i);
		printf(" \n presencia %d",nodo->presencia);
		printf("\n modificado %d",nodo->modificado);
		printf("\n uso %d",nodo->uso);
		printf("\n frame %d",nodo->frame);
		printf("\n indice %d",nodo->unIndice);

		if (nodo->uso == 0 && nodo->modificado == 0 && nodo->presencia == 1) {
			int frame_swap = asignarFrameLibreSWAP();
			cambiar_estado_frame_swap(frame_swap, true);
			frame = nodo->frame;
			void *grabar = malloc(muse_page_size());
			memcpy(grabar, memoria + frame * muse_page_size(),
					muse_page_size());
			//memset(memoria + frame * muse_page_size(),0,1);
			grabar_archivo(path_swap, muse_page_size(),
					muse_page_size() * frame_swap, grabar);
			nodo->frame = frame_swap;
			nodo->presencia = 0;
			nodo->uso = 0;
			nodo->modificado = 0;
			flag_algoritmo = i++;
			break;
		}
		if (nodo->uso == 1 && nodo->modificado == 0 && nodo->presencia == 1) {
					int frame_swap = asignarFrameLibreSWAP();
					cambiar_estado_frame_swap(frame_swap, true);
					frame = nodo->frame;
					void *grabar = malloc(muse_page_size());
					memcpy(grabar, memoria + frame * muse_page_size(),
							muse_page_size());
					grabar_archivo(path_swap, muse_page_size(),
							muse_page_size() * frame_swap, grabar);
					nodo->frame = frame_swap;
					nodo->presencia = 0;
					nodo->uso = 0;
					nodo->modificado = 0;
					flag_algoritmo = i++;
					break;

		}
		if(i == size){
			break;
		}
		i++;
	}
	printf(" \n *****termina while *****");
	printf(" \n *****frame %d *****",frame);
	return frame;
}

void *traer_swap(int frame) {
	cambiar_estado_frame_swap(frame, false);
	return traer_archivo(path_swap, muse_page_size(), frame * muse_page_size());

}

int pagina_segmento(int dir_virtual, int base) {
	return (dir_virtual - base) / muse_page_size();
}

int offset_frame(int pagina, int dir_virtual, int base) {
	return (dir_virtual - base) - (pagina * muse_page_size());
}

void * traer_archivo(char * path, int size, int offset) {
	printf("\n ---- path para traer%s",path);
	printf("\n ---- offset dentro del archivo %d",offset);

	int fd;
	char *data;
	struct stat sbuf;
	void *content = malloc(size);
	if ((fd = open(path, O_RDWR)) == -1) {
		perror("open");
		content = NULL;
	}

	if (stat(path, &sbuf) == -1) {
		perror("stat");
		content = NULL;
	}

	data = (char *) mmap(0, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, 0);

	if (data == (caddr_t) (-1)) {
		perror("mmap");
		content = NULL;
	}
	memcpy(content, data + offset, size);
	return content;

}

void grabar_archivo(char * path, int size, int offset, void *content) {
	int fd;
	char *data;

	printf("\n ---->grabar archivo %s:",path);
	printf("\n ---->grabar offset %d:",offset);

	struct stat sbuf;
	if ((fd = open(path, O_RDWR)) == -1) {
		perror("open");

	}

	if (stat(path, &sbuf) == -1) {
		perror("stat");

	}

	data = (char *) mmap(0, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, 0);

	if (data == (caddr_t) (-1)) {
		perror("mmap");
		content = NULL;
	}

	memcpy(data + offset, content, size);

	munmap(data, sbuf.st_size);
	close(fd);
	if (offset ==0){
			void * contenido = traer_archivo(path_swap,muse_page_size(),0);
			t_heapMetadata*heap = malloc(sizeof(t_heapMetadata));
			memcpy(heap,contenido, sizeof(t_heapMetadata));
			printf("\n ******heap %d****** \n", heap->size);
			memcpy(heap,contenido+12, sizeof(t_heapMetadata));
			printf("\n ******heap %d****** \n", heap->size);
	}

}

int creacionDeArchivoBitmap(int cantidad, char *path) {
	int x = 0;
	FILE *fh = fopen(path, "wb");
	for (int i = 0; i < cantidad; i++) {
		if (fh != NULL) {
			fwrite(&x, sizeof(x), 1, fh);
		}
	}
	fclose(fh);
	return 0;

}

int analizar_nodo_algoritmo(t_nodo_atributo_paginas * nodo) {
	int frame = nodo->frame;
	if (nodo->presencia == 0) {
		frame = asignarFrameLibre();
		nodo->presencia = 1;
		nodo->frame = frame;
	}
	return frame;
}

void crear_archivo_swap (){
	FILE* file_ptr = fopen(path_swap, "w");
	char *write = malloc(muse_swap_size());
	memset(write, '\0',muse_swap_size() );
	fwrite(write,muse_swap_size(),1,file_ptr);
	fclose(file_ptr);
}

void *get_contenido_virtual (int dir_virtual , t_nodo_segmento* nodoSegmento , int size){
	void* get = malloc(size);
	int flag = -1 ;
	int primera_pagina = pagina_segmento(dir_virtual,nodoSegmento->base);
	int ultima_pagina = pagina_segmento(dir_virtual+size,nodoSegmento->base);
	int  i = primera_pagina ;
	int offset;
	int copiar = size;
	int copiado = 0;
	while( flag == -1 && i <=ultima_pagina ){
		t_nodo_pagina* nodoPagina = list_get(nodoSegmento->list_paginas,
				    		i);
		t_nodo_atributo_paginas *nodoAlgormito = nodo_algoritmo(
							nodoPagina->indiceVector);

		int frame = nodoAlgormito->frame;
		if (nodoAlgormito->presencia == 0 && nodoSegmento->tipo == S_ALLOC){
			 int frame = asignarFrameLibre();
			 nodoAlgormito->presencia = 1;
			 void  * pagina = traer_archivo(path_swap, muse_page_size() *nodoAlgormito->frame,muse_page_size() );
			 nodoAlgormito->frame = frame;
			 offset = frame * muse_page_size();
			 memcpy(memoria + offset,pagina, muse_page_size());
		}
		else{
			int pagina =pagina_segmento (dir_virtual,nodoSegmento->base);
			char *unPath = path_segmento(nodoSegmento);
			char *contenido =  traer_archivo(unPath, muse_page_size(), pagina);
			if (nodoAlgormito->presencia == 0 ){
				 int frame = asignarFrameLibre();
				 nodoAlgormito->presencia = 1;
				 nodoAlgormito->frame = frame;
				 offset = frame * muse_page_size();
				 memcpy(memoria + offset,contenido, muse_page_size());
			}
			flag = 0;
		}
		printf("\n----pagina %d",i);
		printf("\n----frame %d",i);

		void *contenido_pagina = malloc (muse_page_size());
		memcpy(contenido_pagina,memoria + frame*muse_page_size(), muse_page_size());
		printf("\n----get %s",(char *)contenido_pagina);
		int tam;
		if (copiar > muse_page_size()){
			copiar = copiar - muse_page_size();
		}
		offset = 0;
		if(i == primera_pagina){
			offset =offset_frame(primera_pagina,dir_virtual,nodoSegmento->base);
			tam = muse_page_size()-offset;

		}
		else
			tam = muse_page_size();
		if(i == ultima_pagina){
			tam = size - copiado ;
			flag = 0;
		}
		printf("\n----tam %d",tam);
		printf("\n----copiado %d",copiado);
		printf("\n----offset %d",offset);
		printf("\n----tam %d",tam);

		memcpy(get+copiado,contenido_pagina + offset,tam);
		printf("\n----get %s",(char *) get);
		copiado = copiado + tam;
		i++;
		free(contenido_pagina);
	}
	return get;
}


t_heapMetadata* obtener_heap(int dir_virtual , t_nodo_segmento* nodoSegmento){

	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
    int pagina = pagina_segmento(dir_virtual,nodoSegmento->base);
	int offset_del_frame = offset_frame(pagina, dir_virtual, nodoSegmento->base);
    t_nodo_pagina* nodoPagina = list_get(nodoSegmento->list_paginas,
						pagina);
	t_nodo_atributo_paginas *nodoAlgormito = nodo_algoritmo(
			nodoPagina->indiceVector);
	printf("\n----->dir_virtual %d \n", dir_virtual);
	printf("----->pagina %d \n", pagina);
	printf("----->offset del frame %d \n", offset_del_frame);
	int frame = nodoAlgormito->frame;
	int offset;
	if (nodoAlgormito->presencia == 0){
		 printf("\n\n----->presencia es 0\n");
		 int frame_libre = aplicar_algoritmo();
		 printf("\n\n----->frame %d para direccion es virtual %d \n",frame_libre,dir_virtual);
		 nodoAlgormito->presencia = 1;
		 void  * pagina = traer_archivo(path_swap,muse_page_size(),muse_page_size() *nodoAlgormito->frame );
		 nodoAlgormito->frame = frame_libre;
		 offset = frame_libre * muse_page_size();
		 memcpy(memoria + offset,pagina, muse_page_size());
		 //test de heap
		 //falta liberar el frame de swap
		 frame = nodoAlgormito->frame;
		 if(frame == 1){
			 t_heapMetadata*heap = malloc(sizeof(t_heapMetadata));
			 memcpy(heap,pagina, sizeof(t_heapMetadata));
			 printf("\n ******heap %d****** \n", heap->size);
			 memcpy(heap,pagina+12, sizeof(t_heapMetadata));
			 printf("\n ******heap %d****** \n", heap->size);
		 }
	}
	offset = frame * muse_page_size() + offset_del_frame;
	printf("\n\n----->frame %d \n", frame);
	printf("----->offset fisico heap %d \n", offset);
	memcpy(heap, memoria + offset, sizeof(t_heapMetadata));
	printf("----->heap size %d \n\n", heap->size);
	//estado_algoritmo();
	return heap;

}

void estado_algoritmo(){
	printf ("\n estado algoritmo");
	for (int i=0;i<list_size(lista_algoritmo);i++){
		t_nodo_atributo_paginas * nodoAlgormito =list_get(lista_algoritmo,i);
		printf ("\n -*-*pagina %d",i);
		printf ("\n -*-*presencia %d",nodoAlgormito->presencia);
		printf("\n *-*-frame %d  \n ",nodoAlgormito->frame);
		printf ("\n -*-* modificado %d",nodoAlgormito->modificado);
		printf("\n *-*-usado %d  \n ",nodoAlgormito->uso);
	}
}

void  copy_contenido_virtual (int dir_virtual , t_nodo_segmento* nodoSegmento , int size,void *contenido){
	int flag = -1 ;
	int primera_pagina = pagina_segmento(dir_virtual,nodoSegmento->base);
	int ultima_pagina = pagina_segmento(dir_virtual+size,nodoSegmento->base);
	int  i = primera_pagina ;
	int offset;
	int copiar = size;
	int copiado = 0;
	while( flag == -1){
		t_nodo_pagina* nodoPagina = list_get(nodoSegmento->list_paginas,
				    		i);
		t_nodo_atributo_paginas *nodoAlgormito = nodo_algoritmo(
							nodoPagina->indiceVector);

		int frame = nodoAlgormito->frame;
		if (nodoAlgormito->presencia == 0 && nodoSegmento->tipo == S_ALLOC){
			 int frame = asignarFrameLibre();
			 nodoAlgormito->presencia = 1;

			 void  * pagina = traer_archivo(path_swap, muse_page_size(),muse_page_size() *nodoAlgormito->frame);
			 nodoAlgormito->frame = frame;
			 offset = frame * muse_page_size();
			 memcpy(memoria + offset,pagina, muse_page_size());
		}
		else{
			int pagina =pagina_segmento (dir_virtual,nodoSegmento->base);
			char *unPath =path_segmento(nodoSegmento) ;
			char *contenido =  traer_archivo(unPath, muse_page_size(), pagina);
			if (nodoAlgormito->presencia == 0 ){
			    int frame = asignarFrameLibre();
				nodoAlgormito->presencia = 1;
				nodoAlgormito->frame = frame;
				offset = frame * muse_page_size();
				memcpy(memoria + offset,contenido, muse_page_size());
			}
		}
		printf("\n----pagina %d",i);
		printf("\n----frame %d",i);

		int tam;
		if (copiar > muse_page_size()){
			copiar = copiar - muse_page_size();
		}
		offset = 0;
		if(i == primera_pagina){
			offset =offset_frame(primera_pagina,dir_virtual,nodoSegmento->base);
			tam = muse_page_size()-offset;

		}
		else
			tam = muse_page_size();
		if(i == ultima_pagina){
			tam = size -copiado ;
			flag = 0;
		}

		printf("\n----copiado %d",copiado);
		printf("\n----offset %d",offset);
		printf("\n----frame %d",frame);
		printf("\n----tam %d",tam);


		printf("\n----copiar en la direccion %d",offset+frame*muse_page_size());

		memcpy(memoria+offset+frame*muse_page_size(),contenido + copiado,tam);

		copiado = copiado + tam;
		i++;
	}
}


void estado_heap(t_nodo_segmento* nodoSegmento){
		int desde = nodoSegmento->base;
		int hasta = nodoSegmento->base + nodoSegmento->tamanio;
		int primera_pagina = (desde / muse_page_size());
		int ultima_pagina = list_size(nodoSegmento->list_paginas);
		t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));

		int offset = 0;
		int pagina = 0;
		int dir_virtual = nodoSegmento->base;
		do {
			pagina = pagina_segmento(dir_virtual, nodoSegmento->base);
			heap = obtener_heap(dir_virtual , nodoSegmento);
			offset = offset + 5;
			printf("****estado heap ****** \n");
			printf("****dir virtual antes de la estructura: %d****** \n", dir_virtual);
			printf("****dir virtual despues de la estructura: %d****** \n", offset);
			printf("****heap size %d \n", heap->size);
			offset = offset + heap->size;
			dir_virtual = offset;


		} while (offset < (nodoSegmento->base + nodoSegmento->tamanio) - 1);


}


int agregar_paginas (int bytes_faltan ,t_nodo_segmento* nodoSegmento){
		int paginas_necesarias;
		if (bytes_faltan % muse_page_size() == 0) {
			paginas_necesarias = bytes_faltan / muse_page_size();
		} else {
			paginas_necesarias = bytes_faltan / muse_page_size() + 1;
		}

		int cant_paginas = list_size(nodoSegmento->list_paginas);

		printf(" \n cantidad de paginas necesarias %d \n", paginas_necesarias);
		printf(" \n cantidad de paginas antes %d \n",
				list_size(nodoSegmento->list_paginas));

		int unframeLibre;

		for (int i = 0; i < paginas_necesarias; i++) {
			int unframeLibre = asignarFrameLibre();
			int indiceLibre = asignarIndiceVectorLibre();

			t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
			nodo_pagina->indiceVector = indiceLibre;

			crear_nodo_indice_algoritmo(indiceLibre, unframeLibre, 1);
			printf(" \n -------indice %d \n", indiceLibre);
			cambiar_estado_frame(unframeLibre, true);
			printf(" \n -------frame %d \n", unframeLibre);

			list_add(nodoSegmento->list_paginas, nodo_pagina);
		}
		return paginas_necesarias;

}

void actualizar_heap(t_heapMetadata* heap,int offset,t_nodo_segmento* segmento){
		printf("\n ******direccion virtual a actualizar %d****** \n", offset);

		int pagina =  pagina_segmento (offset, segmento->base);
		int offset_del_frame = offset_frame(pagina, offset,
				segmento->base);
		t_nodo_pagina* nodoPagina = list_get(segmento->list_paginas, pagina);
		t_nodo_atributo_paginas *nodoAlgormito = nodo_algoritmo(
				nodoPagina->indiceVector);

		int frame = nodoAlgormito->frame;
		int frame_libre;

		if (nodoAlgormito->presencia == 0) {
			frame_libre = aplicar_algoritmo();
			cambiar_estado_frame(frame_libre, true);
			void * pagina_swap = traer_swap(frame);
			//memset(memoria+ frame_libre * muse_page_size(),0,muse_memory_size());
			memcpy(memoria + frame_libre * muse_page_size(), pagina_swap,
						muse_page_size());
			frame = frame_libre;
			free(pagina_swap);
		}
		nodoAlgormito->presencia = 1;
		nodoAlgormito->modificado = 0;
		nodoAlgormito->frame = frame;

		offset = frame * muse_page_size() + offset_del_frame;
		printf("\n******actualizo heap****** \n", frame);
		printf("******frame %d****** \n", frame);
		printf("******pagina %d****** \n", pagina);
		printf("******heap %d****** \n", heap->size);
		printf("******direccion virtual del heap %d****** \n", offset);
		memcpy(memoria + offset, heap, sizeof(t_heapMetadata));
}

void test_swap(){
	grabar_archivo(path_swap, muse_page_size(), 0, memoria);
	void * contenido = traer_archivo(path_swap,muse_page_size(),0);
	t_heapMetadata*heap = malloc(sizeof(t_heapMetadata));
	memcpy(heap,contenido, sizeof(t_heapMetadata));
	printf("\n ******heap %d****** \n", heap->size);
	memcpy(heap,contenido+12, sizeof(t_heapMetadata));
	printf("\n ******heap %d****** \n", heap->size);
}
