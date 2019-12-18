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


void muse_init();
typedef struct {
	int id;
	t_list *list_segmento;
} t_nodo_proceso;

typedef enum {
	S_ALLOC,
	S_MAP_PRIVATE,
	S_MAP_SHARED
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
} tipo_map_shared;

typedef struct {
	char *path;
} tipo_map_private;


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

typedef struct {
   int unIndice;
   int frame;
   bool presencia;
   int uso;
   int modificado;
}__attribute__((packed))
t_nodo_atributo_paginas;



int cantidad_paginas_totales;
int cantidad_frames;
int cantidad_frames_swap;


t_bitarray *bitarray_swap;
t_bitarray *bitarray;

int muse_socket;
void* memoria ;

int indice;

t_list *lista_algoritmo;

t_vector_frames *vectorFrames;
t_vector_atributo_paginas  *vectorAtributoPaginas;
void estadoFrames();
char * path_swap = "muse.swap";

bool existe_memoria_para_frames(int cantidad_paginas_necesarias);
bool estaOcupada(int pag);

void cambiar_estado_frame(int pagina,bool estado);
void cambiar_estado_frame_swap(int frame, bool estado);

int asignar_dir_memoria(t_nodo_segmento* nodoSegmento,uint32_t recervar);
int asignarIndiceVectorLibre();

t_nodo_proceso *crear_nodo(int id);
t_nodo_proceso* existe_proceso_en_lista(int id);
t_nodo_proceso* procesar_id(int id);
t_nodo_proceso* existe_proceso_en_lista(int id);

int asignarFrameLibre();
int asignarFrameLibreSWAP();

t_nodo_segmento* procesar_segmentacion(t_list* listaSegmento);
t_nodo_segmento* crear_nodo_segmento();

int recorer_segmento_espacio_libre(t_nodo_segmento* nodoSegmento,uint32_t memoria_reservar);
int agrandar_segmento(t_nodo_segmento* nodoSegmento,uint32_t memoria_reservar);

t_nodo_segmento *buscar_segmento(uint32_t src,t_list *list);
char * path (t_nodo_segmento* segmento) ;

t_bitarray *crearBitmap(int cantidadDepagina,int diferencia,char *path);
int frameLibre();
int frameLibreSWAP();

void sincronizar(t_nodo_segmento *segmento,size_t size);

void crear_nodo_indice_algoritmo(int indice,int frame,int presencia);

t_nodo_atributo_paginas * nodo_algoritmo(int unIndice);
int aplicar_algoritmo();
void flush(int pagina,char *contenido_pagina,char *path);
void *traer_swap(int frame);
char * path_segmento (t_nodo_segmento* segmento) ;

int pagina_segmento (int dir_virtual,int base);
int offset_frame(int pagina,int dir_virtual,int base);

void estado_bit_array();

int creacionDeArchivoBitmap(int cantidad,char *path);

char *path_bitmap;
void grabar_archivo(char * path, int size , int offset, void *content);
void * traer_archivo(char * path, int size , int offset);

int analizar_nodo_algoritmo(t_nodo_atributo_paginas * nodo);
void crear_archivo_swap ();
#endif /* MUSE_H_ */
