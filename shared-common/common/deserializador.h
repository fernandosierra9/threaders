#ifndef LIBS_DESERIALIZADOR_H_
#define LIBS_DESERIALIZADOR_H_

int recv_package(int fd, void* receiver, size_t size);

int recv_package_variable(int fd, void** receiver);

#endif
