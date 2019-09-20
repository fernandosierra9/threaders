#ifndef LIBS_SERIALIZADOR_H_
#define LIBS_SERIALIZADOR_H_

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "protocols.h"

void* serializer_serialize_package(t_package* package, int bytes);

#endif
