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

			//Crea un nodo proceso si no existe
			t_nodo_proceso* nodoProceso = procesar_id(
					malloc_receive->id_libmuse);


			t_malloc_ok* res = malloc(sizeof(t_malloc_ok));

			//Se fija si existe segmentos
			int cantidad_segmentos=list_size(nodoProceso->list_segmento);
			if(cantidad_segmentos==0){
				t_nodo_segmento* nodo_Segmento = crear_nodo_segmento();

				res->ptr = asignar_dir_memoria(nodo_Segmento, malloc_receive->tam);
				list_add(nodoProceso->list_segmento, nodo_Segmento);
			}
			else{
				t_nodo_segmento* nodoSegmento=list_get(nodoProceso->list_segmento,cantidad_segmentos-1);
				int resp =recorer_segmento_espacio_libre(nodoSegmento,malloc_receive->tam);

				//agrandar segmento
				if (resp == -1){
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
			// TODO: Implementation
		}
		case MAP: {
			// TODO: Implementation
		}
		case UNMAP: {
			// TODO: Implementation
		}
		case READ: {
			muse_logger_info("***READ RECIBIDO***");
			t_read* read_recv = utils_receive_and_deserialize(libmuse_fd, protocol);
			//char * test;
			//test = malloc(strlen("fernando")+1);
			//memcpy(test,read_recv->pathname,strlen("fernando")+1);
			//muse_logger_info("****Process with pid %d*****", read_recv->id_sac_cli);
			muse_logger_info("****Process with pid %s*****",read_recv->pathname);

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
	int frameLibre = -1;
	for (int i = 0; i < cantidad_paginas_totales; i++) {
		if (vectorFrames[i].libre) {
			frameLibre = i;
			break;
		}
	}
	return frameLibre;

}


int asignarIndiceVectorLibre() {
	int paginaLibre = -1;
	for (int i = 0; i < cantidad_paginas_totales; i++) {
		if (vectorAtributoPaginas[i].libre) {
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
	int memoria_reservar2  = memoria_reservar +5;
	int cantidad_paginas_necesarias;
	int pagina = -1;
	if (list_size(nodoSegmento->list_paginas) == 0) {
		cantidad_paginas_necesarias = (memoria_reservar2 / muse_page_size())
				+ 1;
		int primerFrameLibre;
		int frameLibre;
		if (existe_memoria_parar_paginas(cantidad_paginas_necesarias)) {
			for (int i = 0; i < cantidad_paginas_necesarias; i++) {

				frameLibre = asignarFrameLibre();
				int indiceLibre = asignarIndiceVectorLibre();

				//printf("frame libre: %d \n",frameLibre);
				//printf("indice libre para pagina: %d \n",indiceLibre);


				cambiar_estado_pagina(frameLibre, false);

				t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
				nodo_pagina->indiceVector = indiceLibre;
				vectorAtributoPaginas[indiceLibre].frame= frameLibre;
				vectorAtributoPaginas[indiceLibre].libre =false;

				list_add(nodoSegmento->list_paginas, nodo_pagina);
				if(i==0){
					primerFrameLibre = frameLibre;
				}


			}

			t_list *lista_frames_proceso = list_create();
			for(int i=0;i<list_size(nodoSegmento->list_paginas);i++){
				t_nodo_pagina* nodoPagina =list_get(nodoSegmento->list_paginas,i);
				t_frames* frame = malloc(sizeof(t_frames));
				//printf("\n indice vector: %d ",nodoPagina->indiceVector);

				frame->nroFrame = vectorAtributoPaginas[nodoPagina->indiceVector].frame;
				list_add(lista_frames_proceso,frame);
			}

			nodoSegmento->tamanio = cantidad_paginas_necesarias * muse_page_size() -1;
			t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
			heap->libre = false;
			heap->size = memoria_reservar;

			printf("offset   %d \n",primerFrameLibre * muse_page_size());
			printf("heap size %d \n",heap->size);

			memcpy(memoria + primerFrameLibre * muse_page_size(), heap,
					sizeof(t_heapMetadata));

			int bytes_sobran = cantidad_paginas_necesarias * muse_page_size() - memoria_reservar -5 ;
			int dir_proxima_estructura  = frameLibre * muse_page_size() + (muse_page_size() - bytes_sobran) ;

			heap->libre = true;
			heap->size  = bytes_sobran -5 -1 ;

			memcpy(memoria + dir_proxima_estructura,
					heap, sizeof(t_heapMetadata));

			printf("offset   %d \n",dir_proxima_estructura);
			printf("heap size %d \n",heap->size);

			printf("cantidad de paginas %d \n",cantidad_paginas_necesarias);
			printf("tamanio de cantidad de paginas %d \n",cantidad_paginas_necesarias*muse_page_size());
			printf("contenido de base segmento %d \n",nodoSegmento->base);
			printf("bytes sobran sin la estructura %d \n", bytes_sobran  );
			printf("bytes sobran con la estructura %d \n", bytes_sobran -5);
			printf("memoria a reservar %d \n",memoria_reservar);
			printf("ultimo frame %d \n",frameLibre);
			printf("ubicacion logica siguiente estructura %d \n" ,nodoSegmento->base + memoria_reservar +5);
			printf("ubicacion fisica siguiente estructura %d \n" ,dir_proxima_estructura);
			printf("desplazamiento dentro del frame %d \n" ,bytes_sobran);

		}
		else {
			return -1;
		}

	}


	return 5;
}

int agrandar_segmento(t_nodo_segmento* nodoSegmento,
		uint32_t memoria_reservar){
	int desde = nodoSegmento->base;
	int hasta = nodoSegmento->base + nodoSegmento->tamanio;

	int primera_pagina = (desde / muse_page_size());
	int ultima_pagina = ((hasta - desde) / muse_page_size()) + 1;

	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));

	int offset = 0;
	int pagina = 0;
	int offset_frame =0;

	t_nodo_pagina* nodoPagina =list_get(nodoSegmento->list_paginas,pagina);
	int frame = vectorAtributoPaginas[nodoPagina->indiceVector].frame;
	offset = frame * muse_page_size();

	do {
		memcpy(heap, memoria + offset, sizeof(t_heapMetadata));
		offset = offset + 5;
		//printf(" \n offset despues de leer estructura %d \n", offset);

		offset = offset + heap->size;
		pagina = (offset - nodoSegmento->base) / muse_page_size() ;
		offset_frame = (offset - nodoSegmento->base) - (pagina*muse_page_size());


	}while (offset <= ultima_pagina*muse_page_size()-1);

	//printf(" \n offset desplazado %d \n", offset);

	offset =  offset - heap->size -5;

	//printf(" \n offset de la estructura anterior %d \n", offset);
	//ultima estructura libre

	int bytes_libres = heap->size;
	int bytes_faltan = (memoria_reservar+5) - bytes_libres;

	int paginas_necesarias = (bytes_faltan / muse_page_size())+1;

	int cant_paginas = list_size(nodoSegmento->list_paginas);

	//int ultima_pagina = cant_paginas -1;

	for(int i=0;i<cant_paginas;i++){
		int frameLibre = asignarFrameLibre();
		int indiceLibre = asignarIndiceVectorLibre();
		t_nodo_pagina *nodo_pagina = malloc(sizeof(t_nodo_pagina));
		nodo_pagina->indiceVector = indiceLibre;
		vectorAtributoPaginas[indiceLibre].frame= frameLibre;
		vectorAtributoPaginas[indiceLibre].libre =false;
		cambiar_estado_pagina(frameLibre, false);
		list_add(nodoSegmento->list_paginas, nodo_pagina);
	}

	nodoSegmento->tamanio = nodoSegmento->tamanio + paginas_necesarias*muse_page_size();

	int total = heap->size +  paginas_necesarias*muse_page_size();

	heap->size = memoria_reservar;
	heap->libre = false;

	int respuesta = offset +5;

	memcpy(memoria + offset,heap, sizeof(t_heapMetadata));

	heap->size = total - memoria_reservar -5;
	heap->libre = true;


	printf("\n ******bytes libres %d****** \n",heap->size);

	offset = offset + memoria_reservar;
	memcpy(memoria + offset,heap, sizeof(t_heapMetadata));

	return respuesta;


}


int recorer_segmento_espacio_libre(t_nodo_segmento* nodoSegmento,
		uint32_t memoria_reservar) {
	int desde = nodoSegmento->base;
	int hasta = nodoSegmento->base + nodoSegmento->tamanio;

	int primera_pagina = (desde / muse_page_size());

	int ultima_pagina = ((hasta - desde) / muse_page_size()) + 1;

	//int offset = primera_pagina*muse_page_size();
	int respuesta = -1;
	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
	//printf("\n base segmento %d \n",nodoSegmento->base);
	//printf("limite segmento %d \n",ultima_pagina*muse_page_size()-1);

	int offset = 0;
	int pagina = 0;
	int offset_frame =0;
	t_nodo_pagina* nodoPagina =list_get(nodoSegmento->list_paginas,pagina);
	int frame = vectorAtributoPaginas[nodoPagina->indiceVector].frame;
	offset = frame * muse_page_size();
	do {
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
				nuevo_heap->libre = 0;
				nuevo_heap->size = memoria_reservar;
				//printf("dir actual: %d \n", offset);
				offset = offset -5 ;
				memcpy(memoria + offset, nuevo_heap,sizeof(t_heapMetadata));

				memcpy(heap,memoria + offset ,sizeof(t_heapMetadata));

				if(heap->libre == false){
					printf("****offset: %d****** \n", offset);
					printf("****size dir actual: %d****** \n", heap->size);
				}

				offset = offset + 5 + nuevo_heap->size ;

				//printf("dir proxima: %d \n", offset);

				heap->libre = true;
				heap->size  = size_heap - memoria_reservar -5 -5 ;

				memcpy(memoria + offset,heap, sizeof(t_heapMetadata));

				printf("******espacio libre : %d***** \n", heap->size);

				break;
			}
		}
		offset = offset + heap->size;
		//estas 2 lineas se usan para memoria virtual
		pagina = (offset - nodoSegmento->base) / muse_page_size() ;
		offset_frame = (offset - nodoSegmento->base) - (pagina*muse_page_size());

		//printf("offset despues de leer el size de  la estructura %d \n",offset);

	}while (offset <= ultima_pagina*muse_page_size()-1);


	return respuesta;
}

bool existe_memoria_parar_paginas(int cantidad_paginas_necesarias) {
	int i = 0;
	int contador = 0;
	while (i < cantidad_paginas_totales) {

		if (vectorFrames[i].libre) {
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

void cambiar_estado_pagina(int frame, bool estado) {
	vectorFrames[frame].libre = estado;
}

bool estaOcupada(int frame)
{
	return vectorFrames[frame].libre;
}

void estadoFrames(){
	for(int i=0;i<cantidad_paginas_totales;i++){
		if(vectorFrames[i].libre){
				//printf("\n posicion %d libre",i);
				vectorFrames[i].libre =false;
		}
	}
	for(int i=0;i<cantidad_paginas_totales;i++){
		if(vectorFrames[i].libre){
				//printf("\n cambiado estado %d libre",i);
		}
	}
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

