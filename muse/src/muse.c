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
			t_malloc_ok* res = malloc(sizeof(t_malloc_ok));
			int cantidad_segmentos=list_size(nodoProceso->list_segmento);
			if(cantidad_segmentos==0){
				t_nodo_segmento* nodo_Segmento = crear_nodo_segmento();

				res->ptr = asignar_dir_memoria(nodo_Segmento, malloc_receive->tam);
				list_add(nodoProceso->list_segmento, nodo_Segmento);
			}
			else{
				t_nodo_segmento* nodoSegmento=list_get(nodoProceso->list_segmento,cantidad_segmentos-1);
				res->ptr =recorer_segmento_espacio_libre(nodoSegmento,malloc_receive->tam);
			}

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

			memcpy(content, memoria + get_receive->src,get_receive->size );


			if (strlen(content) != 0)
			{
				// Response logic
				t_get_ok* response = malloc(sizeof(t_get_ok));
				response->res = content;
				response->tamres = strlen(content)* sizeof(char);
				t_protocol get_protocol = GET_OK;
				utils_serialize_and_send(libmuse_fd, get_protocol, response);
			}
			else {
				 t_protocol fallo = SEG_FAULT;
				 send(libmuse_fd, &fallo, sizeof(int), 0);
				 //send(libmuse_fd,fallo,sizeof(int),0);
			}
			// Else
			break;
		}

		case COPY: {
			muse_logger_info("Copy received");
			t_copy* cpy = utils_receive_and_deserialize(libmuse_fd, protocol);
			muse_logger_info("ver cpy bien ");
			muse_logger_info(
					"Process with pid; %d is trying to copy %d bytes to direction: %d",
					cpy->self_id, cpy->size, cpy->dst);
			muse_logger_info("recibio bien ");



			/*
			int valor;
			memcpy(&valor,cpy->content,4);

			muse_logger_info("id recibi %d", valor);
			*/

			char *test = malloc(cpy->size);
			memcpy(test,cpy->content,cpy->size);

			printf("test %s",test);

			muse_logger_info("test de char %s",test);


			muse_logger_info("id proceso %d", cpy->self_id);

			t_nodo_proceso* nodo = procesar_id(cpy->self_id);

			// Verifico si puedo copiar algo a la posicion solicitada

			t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
			memcpy(heap, memoria + cpy->dst -5, sizeof(t_heapMetadata));

			if (!heap->libre)
			{
				int pagina = cpy->dst / muse_page_size() +1;
				heap->libre = false;
				heap->size = cpy->size;
				memcpy (memoria + cpy->dst-5, heap, sizeof(t_heapMetadata));
				memcpy (memoria + cpy->dst, cpy->content, cpy->size);


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

				printf("frame libre: %d \n",frameLibre);
				printf("indice libre para pagina: %d \n",indiceLibre);


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
				printf("\n indice vector: %d ",nodoPagina->indiceVector);

				frame->nroFrame = vectorAtributoPaginas[nodoPagina->indiceVector].frame;
				list_add(lista_frames_proceso,frame);
			}

			nodoSegmento->tamanio = cantidad_paginas_necesarias * muse_page_size() -1;
			t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
			heap->libre = false;
			heap->size = memoria_reservar;
			memcpy(memoria +primerFrameLibre * muse_page_size(), heap,
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

			//test de recorrido de segmento

			memoria_reservar = 5;
			recorer_segmento_espacio_libre(nodoSegmento,memoria_reservar);
			//pagina = nodoSegmento->base+5;

		} else {
			return -1;
		}

	}
//falta logica else
//buscar en segmento espacio libre
//buscar si segmento se puede agrandar
//crear nuevo segmento
	return nodoSegmento->base +5;
}

int recorer_segmento_espacio_libre(t_nodo_segmento* nodoSegmento,uint32_t memoria_reservar){
	int desde = nodoSegmento->base;
	int hasta= nodoSegmento->base + nodoSegmento->tamanio;

	int primera_pagina = (desde / muse_page_size());

	int ultima_pagina = (hasta/ muse_page_size())+1;

	//int offset = primera_pagina*muse_page_size();
	int respuesta = -1;
	t_heapMetadata *heap = malloc(sizeof(t_heapMetadata));
	printf("\n base segmento %d \n",nodoSegmento->base);
	printf("limite segmento %d \n",ultima_pagina*muse_page_size()-1);

	/*
	t_list *lista_frames_proceso = list_create();
	for(int i=0;i<list_size(nodoSegmento->list_paginas);i++){
		t_nodo_pagina* nodoPagina =list_get(nodoSegmento->list_paginas,i);
		t_frames* frame = malloc(sizeof(t_frames));
		printf("\n indice vector %d :",nodoPagina->indiceVector);
		frame->nroFrame = vectorAtributoPaginas[nodoPagina->indiceVector].frame;
		list_add(lista_frames_proceso,frame);
	}

	for(int i=0;i<list_size(lista_frames_proceso);i++){
		t_frames* frame =list_get(lista_frames_proceso,i);
		printf("\n nro frama del proceso %d :",frame->nroFrame);
	}
  */

	int offset = 0;
	int pagina = 0;
	int offset_frame =0;
	do{

		t_frames *frame = list_get(nodoSegmento->list_paginas,pagina);
		memcpy(heap,memoria+frame->nroFrame*muse_page_size()+offset_frame,sizeof(t_heapMetadata));
		offset =offset + 5;
		printf(" \n offset despues de leer estructura %d \n",offset);
		printf(" \n %d \n",offset);
		if(heap->libre && memoria_reservar < heap->size){
			printf("\n **** esta libre a partir de este lugar %d ***** \n",offset);
			int tamanio = heap->size;
			heap->libre = false;
			heap->size = memoria_reservar;

			heap->libre = false;
			heap->size = memoria_reservar;
			memcpy(memoria+frame->nroFrame*muse_page_size()+offset_frame, heap,
					sizeof(t_heapMetadata));

			heap->libre = true;
			heap->size = tamanio - memoria_reservar -5;
			memcpy(memoria + frame->nroFrame*muse_page_size()+offset_frame+5+memoria_reservar,
								heap, sizeof(t_heapMetadata));



			respuesta = offset;
			return offset ;
		}
		offset = offset + heap->size;
		pagina = (offset - nodoSegmento->base) / muse_page_size() ;
		offset_frame = (offset - nodoSegmento->base) - (pagina*muse_page_size());

		printf(" \n memoria reservar %d \n",memoria_reservar);
		printf(" \n heap size de la estructura %d \n",heap->size);

		printf("offset despues de leer el size de  la estructura %d \n",offset);

	}while (offset <= ultima_pagina*muse_page_size()-1);

	printf("valor respuesta: %d \n",respuesta);


	return respuesta;
}

bool existe_memoria_parar_paginas(int cantidad_paginas_necesarias) {
	int i = 0;
	int contador =0;
	while (i < cantidad_paginas_totales) {

		if (vectorFrames[i].libre) {
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
				printf("\n posicion %d libre",i);
				vectorFrames[i].libre =false;
		}
	}
	for(int i=0;i<cantidad_paginas_totales;i++){
		if(vectorFrames[i].libre){
				printf("\n cambiado estado %d libre",i);
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

