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
			int cantidad_segmentos=list_size(nodoProceso->list_segmento);
			if(cantidad_segmentos==0){
				t_nodo_segmento* nodo_Segmento = crear_nodo_segmento();
				nodo_Segmento->tipo = S_ALLOC;
				nodo_Segmento->map = NULL;
				res->ptr = asignar_dir_memoria(nodo_Segmento, malloc_receive->tam);
				list_add(nodoProceso->list_segmento, nodo_Segmento);
			}
			else{
				t_nodo_segmento* nodoSegmento;
				int i ;
				int resp;
				for(i=0;i<cantidad_segmentos;i++){
					nodoSegmento=list_get(nodoProceso->list_segmento,i);
					resp=-1;
					if (nodoSegmento->tipo == S_ALLOC){
						resp =recorer_segmento_espacio_libre(nodoSegmento,malloc_receive->tam);

					}
					if (resp != -1 ){
						break;
					}
				}
				//agrando el ultimo alloc si no se pudo agrandar buscar un espacio libre
				if(resp ==-1){
					resp = agrandar_segmento(nodoSegmento,malloc_receive->tam);
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

//			char* content = malloc(sizeof(get_receive->size));
//			memcpy(content, memoria + get_receive->src,get_receive->size );

			if (!existe_proceso_en_lista(get_receive->id_libmuse)) {
				t_protocol get_failed = SEG_FAULT;
				send(libmuse_fd, &get_failed, sizeof(t_protocol), 0);
				break;
			}

			else {

				char* content = malloc(get_receive->size);
				memcpy(&content, memoria + get_receive->src, sizeof(content));

				t_get_ok* response = malloc(sizeof(t_get_ok));
				response->res = content;
				response->tamres = sizeof(get_receive->size);
				t_protocol get_protocol = GET_OK;
				utils_serialize_and_send(libmuse_fd, get_protocol, response);
				break;
			}
		}

		case COPY: {
			muse_logger_info("Copy received");
			t_copy* cpy = utils_receive_and_deserialize(libmuse_fd, protocol);
			muse_logger_info(
					"Process with pid; %d is trying to copy %d bytes to direction: %d",
					cpy->self_id, cpy->size, cpy->dst);

			/*
			int valor;
			memcpy(&valor,cpy->content,4);

			muse_logger_info("id recibi %d", valor);
			*/

			muse_logger_info("id proceso %d", cpy->self_id);
			t_nodo_proceso* nodo = procesar_id(cpy->self_id);
			t_protocol cpy_protocol;

			if (!existe_proceso_en_lista(cpy->self_id)) {
				t_copy_response* copy_res = malloc(sizeof(t_copy_response));
				copy_res->res = -1;
				cpy_protocol = SEG_FAULT;
				utils_serialize_and_send(libmuse_fd, cpy_protocol, copy_res);
				break;
			}

			else {
				t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
				memcpy(heap, memoria + cpy->dst - 5, sizeof(t_heapMetadata));

				if (!heap->libre) {
					heap->size = cpy->size;
					memcpy(memoria + cpy->dst - 5, heap,
							sizeof(t_heapMetadata));
					memcpy(memoria + cpy->dst, cpy->content, cpy->size);
				}

				else
				{
					t_copy_response* copy_res = malloc(sizeof(t_copy_response));
					copy_res->res = -1;
					cpy_protocol = SEG_FAULT;
					utils_serialize_and_send(libmuse_fd, cpy_protocol, copy_res);
					break;
				}

//				int val;
//				memcpy(&val, cpy->content, sizeof(int));
//				printf("Received %d", val);

				t_copy_response* copy_res = malloc(sizeof(t_copy_response));
				copy_res->res = 1;
				t_protocol cpy_protocol = GET_OK;
				utils_serialize_and_send(libmuse_fd, cpy_protocol, copy_res);
				break;
			}

		}
		case SYNC: {
			muse_logger_info("SYNC received\n");
			t_msync *msync_receive = utils_receive_and_deserialize(libmuse_fd,
											protocol);
			muse_logger_info("id proceso %d", msync_receive->id_libmuse);
			t_nodo_proceso* nodoProceso = procesar_id(
					msync_receive->id_libmuse);
			t_nodo_segmento * segmento = buscar_segmento(msync_receive->src,nodoProceso->list_segmento);
			t_protocol protocol = SYNC_OK;
			if(msync_receive->src+ msync_receive->size <= segmento->tamanio +segmento->base ){

				sincronizar(segmento,msync_receive->size);
			}
			send(libmuse_fd,&protocol,sizeof(protocol),0);
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
		    if(map_receive->flag == MAP_PRIVATE){
		    	muse_logger_info("**** MAP privado *****");
		    	nodo_Segmento->tipo = MAP_PRIVATE;
		    	tipo_map_private *type = malloc(sizeof(tipo_map_private));
		    	type->path = strdup(map_receive->path);
		    }
		    if(map_receive->flag == MAP_SHARED){
		 		muse_logger_info("**** MAP compartido *****");
		 		nodo_Segmento->tipo = MAP_SHARED;
		 		tipo_map_shared *type = malloc(sizeof(tipo_map_shared));
		 		type->cant_links = 1;
		 		type->path = strdup(map_receive->path);
		 		nodo_Segmento->map = type;
		    }

		    t_nodo_proceso* nodoProceso = procesar_id(
		    map_receive->id_libmuse);

		    int cantidad_segmentos=list_size(nodoProceso->list_segmento);

		    int cantidad_paginas = (map_receive->size/muse_page_size()+1);
		    int tamanio = cantidad_paginas*muse_page_size();
		    if(cantidad_segmentos==0){

		    	nodo_Segmento->base =0;
		    	nodo_Segmento->tamanio = tamanio;
		    	list_add(nodoProceso->list_segmento, nodo_Segmento);
		    }
		    else{
		        t_nodo_segmento* otroNodoSegmento=list_get(nodoProceso->list_segmento,cantidad_segmentos-1);
		        nodo_Segmento->base = otroNodoSegmento->base + otroNodoSegmento->tamanio +1 ;
		    }
		    nodo_Segmento->list_paginas = list_create();
		    for(int i=0;i<cantidad_paginas;i++){
		    	t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
		    	nodo_pagina->indiceVector = asignarIndiceVectorLibre();
		    	list_add(nodo_Segmento->list_paginas,nodo_pagina);
		    	//no necesito un frame
		    	crear_nodo_indice_algoritmo(nodo_pagina->indiceVector,-1,0);
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
	lista_algoritmo = list_create();
	memoria = malloc(muse_memory_size());
	int cantidad_paginas_reales = muse_memory_size() / muse_page_size();
	int diferencia= cantidad_frames % 8;
	cantidad_frames = cantidad_paginas_reales;
	crearBitmap(cantidad_frames,diferencia);
	int cantidad_paginas_virtuales = muse_swap_size() / muse_page_size();
	cantidad_paginas_totales = cantidad_paginas_reales
			+ cantidad_paginas_virtuales;

	vectorFrames = (t_vector_frames*)calloc(cantidad_paginas_totales, sizeof(t_vector_frames));
	vectorAtributoPaginas = (t_vector_atributo_paginas*)calloc(cantidad_paginas_totales, sizeof(t_vector_atributo_paginas));

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
int asignarFrameLibre() {
	return frameLibre();

}


int asignarIndiceVectorLibre() {
	indice ++;
	return indice -1;
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
	int memoria_reservar2  = memoria_reservar +5;
	int cantidad_paginas_necesarias;
	int pagina = -1;
	if (list_size(nodoSegmento->list_paginas) == 0) {
		cantidad_paginas_necesarias = (memoria_reservar2 / muse_page_size())
				+ 1;
		int primerFrameLibre;
		int frameLibre;
		if (existe_memoria_para_frames(cantidad_paginas_necesarias)) {
			pagina =0;
			for (int i = 0; i < cantidad_paginas_necesarias; i++) {

				frameLibre = asignarFrameLibre();
				int indiceLibre = asignarIndiceVectorLibre();

				printf("frame libre: %d \n",frameLibre);
				printf("indice libre para pagina: %d \n",indiceLibre);

				//cambio a ocupado
				cambiar_estado_frame(frameLibre, true);

				t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
				nodo_pagina->indiceVector = indiceLibre;

				//falta c
				crear_nodo_indice_algoritmo(indiceLibre,frameLibre,1);

				list_add(nodoSegmento->list_paginas, nodo_pagina);
				if(i==0){
					primerFrameLibre = frameLibre;
				}

			}
			//ESto se va a usar para calcular donde escribir la estructura
			//ahora hace escritura contigua
			t_list *lista_frames_proceso = list_create();
			for(int i=0;i<list_size(nodoSegmento->list_paginas);i++){
				t_nodo_pagina* nodoPagina =list_get(nodoSegmento->list_paginas,i);
				t_frames* frame = malloc(sizeof(t_frames));
				printf("\n indice vector: %d ",nodoPagina->indiceVector);
				printf("\n size lista algoritmo: %d ",list_size(lista_algoritmo));
				t_nodo_atributo_paginas * nodoAlgoritmo = nodo_algoritmo(nodoPagina->indiceVector);
				frame->nroFrame = nodoAlgoritmo->frame;
				list_add(lista_frames_proceso,frame);
			}

			nodoSegmento->tamanio = cantidad_paginas_necesarias * muse_page_size() -1;

			t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
			heap->libre = false;
			heap->size = memoria_reservar;

			memcpy(memoria + primerFrameLibre * muse_page_size(), heap,
					sizeof(t_heapMetadata));

			int bytes_sobran = cantidad_paginas_necesarias * muse_page_size() - memoria_reservar -5;
			int dir_proxima_estructura  = frameLibre * muse_page_size() + (muse_page_size() - bytes_sobran) ;

			heap->libre = true;
			heap->size  = bytes_sobran -5 -1;

			memcpy(memoria + dir_proxima_estructura,
					heap, sizeof(t_heapMetadata));

			printf("cantidad de paginas %d \n",cantidad_paginas_necesarias);
			printf("tamanio de cantidad de paginas %d \n",cantidad_paginas_necesarias*muse_page_size());
			printf("contenido de base segmento %d \n",nodoSegmento->base);
			printf("bytes sobran sin la estructura %d \n", bytes_sobran );
			printf("bytes sobran con la estructura %d \n", bytes_sobran -5);
			printf("memoria a reservar %d \n",memoria_reservar);
			printf("ultimo frame %d \n",frameLibre);
			printf("ubicacion logica siguiente estructura %d \n" ,nodoSegmento->base + memoria_reservar +5);
			printf("ubicacion fisica siguiente estructura %d \n" ,dir_proxima_estructura);
			printf("desplazamiento dentro del frame %d \n" ,bytes_sobran);

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

int recorer_segmento_espacio_libre(t_nodo_segmento* nodoSegmento,
		uint32_t memoria_reservar)
{
	int desde = nodoSegmento->base;
	int hasta = nodoSegmento->base + nodoSegmento->tamanio;
	int primera_pagina = (desde / muse_page_size());
	int ultima_pagina = list_size(nodoSegmento->list_paginas);
	int respuesta = -1;
	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));

	int offset = 0;
	int pagina = 0;
	int dir_virtual = nodoSegmento->base ;
	do {

		pagina = pagina_segmento (dir_virtual,nodoSegmento->base);
		int offset_del_frame = offset_frame(pagina,dir_virtual,nodoSegmento->base);

		t_nodo_pagina* nodoPagina =list_get(nodoSegmento->list_paginas,pagina);
		t_nodo_atributo_paginas * otroNodoAlgoritmo= nodo_algoritmo(nodoPagina->indiceVector);

		int frame = otroNodoAlgoritmo->frame;
		offset = frame * muse_page_size() + offset_del_frame;

		memcpy(heap, memoria + offset, sizeof(t_heapMetadata));
		offset = offset + 5;
		//printf(" \n frame %d \n", frame);
		printf(" \n offset despues de leer estructura %d \n", offset);
		printf(" \n heap size %d \n", heap->size);

		if (heap->libre == true){
			if ((memoria_reservar+5) < heap->size) {
				respuesta = offset;
				printf("\n **** esta libre a partir de este lugar %d ***** \n", offset);
				int size_heap = heap->size;
				t_heapMetadata *nuevo_heap = malloc(sizeof(t_heapMetadata));
				nuevo_heap->libre = false;
				nuevo_heap->size = memoria_reservar;
				//printf("dir actual: %d \n", offset);
				offset = offset -5 ;
				printf("****offset: %d****** \n", offset);
				printf("****size dir actual: %d****** \n", heap->size);

				memcpy(memoria + offset, nuevo_heap,sizeof(t_heapMetadata));

				offset = offset + 5 + nuevo_heap->size ;

				heap->libre = true;
				heap->size  = size_heap - memoria_reservar -5 ;

				memcpy(memoria + offset,heap, sizeof(t_heapMetadata));

				printf("****libre apartir: %d****** \n", offset+5);
				printf("******espacio libre : %d***** \n", heap->size);

				break;
			}
		}
		offset = offset + heap->size;
		dir_virtual =  offset;

	}while (offset < (nodoSegmento->base+nodoSegmento->tamanio)-1);

	printf("****no se encontro espacio libre en el segmento ***\n");
	return respuesta;
}

bool existe_memoria_para_frames(int cantidad_paginas_necesarias) {
	int i = 0;
	int contador = 0;
	while (i < cantidad_paginas_totales) {

		if (bitarray_test_bit(bitarray,i)==0) {
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
	if ( estado == false) {
		bitarray_clean_bit(bitarray, frame);
	}
	if (estado == true){
		bitarray_set_bit(bitarray, frame);
	}
}

bool estaOcupada(int frame)
{
	return bitarray_test_bit(bitarray, frame);
}

void estadoFrames()
{
	for(int i=0;i<cantidad_frames;i++){
		if(bitarray_test_bit(bitarray, i)){
				printf("\n posicion %d libre",i);
		}
	}
}

int agrandar_segmento(t_nodo_segmento* nodoSegmento,
		uint32_t memoria_reservar){
	int desde = nodoSegmento->base;
	int hasta = nodoSegmento->base + nodoSegmento->tamanio;

	int primera_pagina = (desde / muse_page_size());
	int ultima_pagina = list_size(nodoSegmento->list_paginas) -1;

	int dir_virtual = nodoSegmento->base;
	int dir_fisica = 0;

	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));

	int offset = 0;
	int pagina = primera_pagina;



	do {
		pagina = pagina_segmento (dir_virtual,nodoSegmento->base);
		int offset_del_frame = offset_frame(pagina,dir_virtual,nodoSegmento->base);
		t_nodo_pagina* nodoPagina =list_get(nodoSegmento->list_paginas,pagina);
		t_nodo_atributo_paginas *nodoAlgormito = nodo_algoritmo(nodoPagina->indiceVector);

		if(pagina +1 <= ultima_pagina ) {
			t_nodo_pagina* siguienteNodoPagina =list_get(nodoSegmento->list_paginas,pagina+1);
			t_nodo_atributo_paginas *siguienteNodoAlgormito = nodo_algoritmo(siguienteNodoPagina->indiceVector);
		}

		int frame = nodoAlgormito->frame;
		offset = frame * muse_page_size() + offset_del_frame ;

		memcpy(heap, memoria + offset, sizeof(t_heapMetadata));
		offset = offset + 5;

		offset = offset + heap->size;
		dir_virtual =  offset;

	}while (offset < (nodoSegmento->base+nodoSegmento->tamanio)-1);


	offset =  offset - heap->size -5;


	printf(" \n offset de la estructura libre %d \n", offset);
	printf(" \n bytes libres %d \n", heap->size);

	int bytes_libres = heap->size;
	int bytes_faltan = (memoria_reservar+5) - bytes_libres;

	int paginas_necesarias = (bytes_faltan / muse_page_size())+1;

	int cant_paginas = list_size(nodoSegmento->list_paginas);

	//int ultima_pagina = cant_paginas -1;

	printf(" \n cantidad de paginas necesarias %d \n", paginas_necesarias);

	//agrega los nodos de para algoritmo
	for(int i=0;i<paginas_necesarias;i++){
		int frameLibre = asignarFrameLibre();
		int indiceLibre = asignarIndiceVectorLibre();
		t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
		nodo_pagina->indiceVector = indiceLibre;

		crear_nodo_indice_algoritmo(indiceLibre,frameLibre,1);

		cambiar_estado_frame(frameLibre, true);

		list_add(nodoSegmento->list_paginas, nodo_pagina);
	}

	nodoSegmento->tamanio = nodoSegmento->tamanio + paginas_necesarias*muse_page_size();

	int total = heap->size +  paginas_necesarias*muse_page_size();

	printf(" \n bytes libres + 32 %d \n", total);

	heap->size = memoria_reservar;
	heap->libre = false;

	int respuesta = offset +5;

	memcpy(memoria + offset,heap, sizeof(t_heapMetadata));

	offset = offset + 5 + memoria_reservar;

	heap->size = total - memoria_reservar -5;
	heap->libre = true;


	printf("\n ******bytes libres %d****** \n",heap->size);
	printf("\n ******offset de estructura libre %d****** \n",offset);
	memcpy(memoria + offset,heap, sizeof(t_heapMetadata));

	return respuesta;

}

t_nodo_segmento *buscar_segmento(uint32_t src,t_list *list) {
	int existeNodoEnLaLista(t_nodo_segmento *nodo_proceso) {
			return (nodo_proceso->base + nodo_proceso->tamanio) < src;
	}
	return list_find(list, (void*) existeNodoEnLaLista);
}

void sincronizar(t_nodo_segmento *segmento,size_t size){
	int pagina = 0;
	int copiado = size;
	void *contenido_pagina = malloc(muse_page_size());
	do{
		t_nodo_pagina * nodo_pagina = list_get(segmento->list_paginas,pagina);

		t_nodo_atributo_paginas *nodoAlgoritmo = nodo_algoritmo(nodo_pagina->indiceVector);
		int frame_libre;
		if(nodoAlgoritmo->modificado ==1){
			int frame = nodoAlgoritmo->frame;
			if(	nodoAlgoritmo->presencia ==0){
				frame_libre = asignarFrameLibre();
				if(	frame_libre	< 0	){
					frame_libre = aplicar_algoritmo();
				}
				cambiar_estado_frame(frame_libre,true);
				void * pagina_swap = traer_swap(frame);
				memcpy(memoria+frame_libre*muse_page_size(),pagina_swap,muse_page_size());
				frame = frame_libre;
				free(pagina_swap);
			}
			memcpy(contenido_pagina,memoria+frame_libre*muse_page_size(),muse_page_size());
			nodoAlgoritmo->presencia = 1;
			nodoAlgoritmo->modificado = 0;
			nodoAlgoritmo->frame = frame;
			char *path = path_segmento(segmento);
			flush(pagina,contenido_pagina,path);
		}
		pagina ++;
		copiado = copiado - muse_page_size();
	}while(copiado >0 && (pagina < list_size(segmento->list_paginas)));
}


char * path_segmento (t_nodo_segmento* segmento) {
	if (segmento->tipo == MAP_SHARED){
		char * path = ((tipo_map_shared*) segmento->map)->path;
		return path;
	}
	else{
		char * path = ((tipo_map_private*) segmento->map)->path;
		return path;
	}
}


t_bitarray *crearBitmap(int cantidadDepagina,int diferencia){
		size_t tamanio =cantidadDepagina/8;
		if (diferencia >0){
			tamanio++;

		}
		char bmap[tamanio];

		bzero(bmap,tamanio);

		bitarray = bitarray_create_with_mode(bmap,tamanio, MSB_FIRST);
		size_t	cantidadDebits= bitarray_get_max_bit (bitarray);
		for (int i=0;i<cantidadDebits;i++){
			//printf("*****posicion %d valor %d *****:\n",i,bitarray_test_bit(bitarray,i));

		}
		return bitarray;

}

t_nodo_atributo_paginas * nodo_algoritmo(int unIndice){
	int existeNodoEnLaLista(t_nodo_atributo_paginas *nodo_proceso) {
			return nodo_proceso->unIndice ==  unIndice;
	}
    return list_find(lista_algoritmo, (void*) existeNodoEnLaLista);
}

void crear_nodo_indice_algoritmo(int indice,int frame,int presencia){
	t_nodo_atributo_paginas* nodo = malloc(sizeof(t_nodo_atributo_paginas));
	nodo->frame = frame;
	nodo->modificado = 0;
	nodo->presencia = presencia;
	nodo->unIndice = indice;
	nodo->uso = 0;
	list_add(lista_algoritmo,nodo);

}



int frameLibre(){
	size_t	cantidadDebits= cantidad_frames;
	int i;
	int libre=-1;
	for (i=0;i<cantidadDebits;i++){
		if(bitarray_test_bit(bitarray,i)==0){
			libre=i;
			break;
		}
	}
	return libre;
}

void flush(int pagina,char *contenido_pagina,char *path){

}

int aplicar_algoritmo(){
	return 0;
}

void *traer_swap(int frame){

	return NULL;
}

int pagina_segmento (int dir_virtual,int base){
	return (dir_virtual-base)/muse_page_size();
}

int offset_frame(int pagina,int dir_virtual,int base){
	return (dir_virtual-base) - (pagina *muse_page_size() );
}
