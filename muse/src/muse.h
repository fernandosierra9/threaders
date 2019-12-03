#ifndef MUSE_H_
#define MUSE_H_


#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <stdbool.h>
#include "config/muse_config.h"
#include "logger/muse_logger.h"
#include "../../shared-common/common/sockets.h"
#include "../../shared-common/common/utils.h"


int muse_socket;
void* memoria ;

void muse_init();
typedef struct {
	int id;
	t_list *list_segmento;
} t_nodo_proceso;

typedef enum {
	S_ALLOC,
	S_MAP
}t_segmento;

typedef struct {
	int base;
	int tamanio;
	t_list *list_paginas;
	t_segmento tipo;
	void * map;
} t_nodo_segmento;



typedef struct {
	char *path;
	int cant_links;
} tipo_map;


typedef struct {
	int indiceVector;
} t_nodo_pagina;

t_list *lista_procesos;

typedef struct t_heapMetadata {
	uint32_t size;
	bool libre;
}__attribute__((packed))
t_heapMetadata;


typedef struct {
   bool libre;
}__attribute__((packed))
t_vector_frames;

typedef struct {
   int nroFrame ;
}__attribute__((packed))
t_frames;

typedef struct {
   bool libre;
   int frame;
   bool presencia;
   int uso;
   int modificado;
}__attribute__((packed))
t_vector_atributo_paginas;

int cantidad_paginas_totales;


t_vector_frames *vectorFrames;
t_vector_atributo_paginas  *vectorAtributoPaginas;
void estadoFrames();


bool existe_memoria_parar_paginas(int cantidad_paginas_necesarias);
bool estaOcupada(int pag);

void cambiar_estado_pagina(int pagina,bool estado);

int asignar_dir_memoria(t_nodo_segmento* nodoSegmento,uint32_t recervar);
int asignarIndiceVectorLibre();

t_nodo_proceso *crear_nodo(int id);
t_nodo_proceso* existe_proceso_en_lista(int id);
t_nodo_proceso* procesar_id(int id);
t_nodo_proceso* existe_proceso_en_lista(int id);

int asignarFrameLibre();
t_nodo_segmento* procesar_segmentacion(t_list* listaSegmento);
t_nodo_segmento* crear_nodo_segmento();

int recorer_segmento_espacio_libre(t_nodo_segmento* nodoSegmento,uint32_t memoria_reservar);
int agrandar_segmento(t_nodo_segmento* nodoSegmento,uint32_t memoria_reservar);


#endif /* MUSE_H_ */
