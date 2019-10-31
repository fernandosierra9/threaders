#ifndef MUSE_H_
#define MUSE_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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

typedef struct {
	int base;
	int tamanio;
	t_list *list_paginas;
} t_nodo_segmento;

typedef struct {
	int nro;
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
t_vector_paginas;

int cantidad_paginas_totales;
t_vector_paginas *vectorPaginas;
bool existe_memoria_parar_paginas(int cantidad_paginas_necesarias);

void cambiar_estado_pagina(int pagina,bool estado);
int asignar_dir_memoria(t_nodo_segmento* nodoSegmento,uint32_t recervar);
t_nodo_proceso *crear_nodo(int id);
t_nodo_proceso* existe_proceso_en_lista(int id);
t_nodo_proceso* procesar_id(int id);
t_nodo_proceso* existe_proceso_en_lista(int id);

t_nodo_segmento* procesar_segmentacion(t_list* listaSegmento);
t_nodo_segmento* crear_nodo_segmento();

int recorer_segmento_espacio_libre(t_nodo_segmento* nodoSegmento,uint32_t memoria_reservar);

#endif /* MUSE_H_ */
