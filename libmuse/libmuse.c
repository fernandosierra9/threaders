#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libmuse.h"
#include "../shared-common/common/sockets.h"

int muse_fd;

int muse_init(int id, char* ip, int puerto) {
	muse_fd = socket_connect_to_server(ip, puerto);

	if (muse_fd < 0) {
		puts("Error al conectar con Muse");
		socket_close_conection(muse_fd);
		return -1;
	}
	puts("Conexion con Muse establecida");

	return 0;
}

void muse_close() {
	socket_close_conection(muse_fd);
}

uint32_t muse_alloc(uint32_t tam) {
	t_malloc *malloc_send = malloc(sizeof(t_malloc));
	malloc_send->tam = tam;
	malloc_send->id_libmuse = getpid();
	t_protocol malloc_protocol = MALLOC;
	utils_serialize_and_send(muse_fd, malloc_protocol, malloc_send);

	int received_bytes = recv(muse_fd, &malloc_protocol, sizeof(int), 0);

	switch (malloc_protocol) {
	case MALLOC_OK: {
		t_malloc_ok* res = utils_receive_and_deserialize(muse_fd,
				malloc_protocol);
		return res->ptr;
	}
	case SEG_FAULT: {
		return -1;
	}
	}
}

void muse_free(uint32_t dir) {
	t_free* free_req = malloc(sizeof(t_free));
	free_req->dir = dir;
	free_req->self_id = getpid();
	t_protocol free_protocol = MEMFREE;
	utils_serialize_and_send(muse_fd, free_protocol, free_req);

	int response = recv(muse_fd, &free_protocol, sizeof(t_protocol), 0);

	t_free_response* deserialized_response = utils_receive_and_deserialize(
			muse_fd, free_protocol);

	if (deserialized_response->res == 1) {
		free((void*) dir);
		puts("Operation has been successful");
	}

	else
		puts("Operation failed");
	return;
}

int muse_get(void* dst, uint32_t src, size_t n) {

	t_get* get_send = malloc(sizeof(t_get));
	get_send->id_libmuse = getpid();
	get_send->size = n;
	get_send->src = src;
	t_protocol get_protocol = GET;
	utils_serialize_and_send(muse_fd, get_protocol, get_send);

	int response = recv(muse_fd, &get_protocol, sizeof(t_protocol), 0);

	switch (get_protocol) {
	case GET_OK: {
		t_get_ok* get = utils_receive_and_deserialize(muse_fd, get_protocol);
		memcpy(dst, get->res, get->tamres);
		return 0;
	}

	case SEG_FAULT: {
		return -1;
	}
	}
}

int muse_cpy(uint32_t dst, void* src, int n) {

	t_copy* copy_send = malloc(sizeof(t_copy));
	copy_send->self_id = getpid();
	copy_send->size = n;
	copy_send->dst = dst;
	t_protocol copy_protocol = COPY;
	utils_serialize_and_send(muse_fd, copy_protocol, copy_send);

	int response = recv(muse_fd, &copy_protocol, sizeof(t_protocol), 0);

	t_copy_response* deserialized_res = utils_receive_and_deserialize(muse_fd,
			copy_protocol);

	if (deserialized_res->res == 1) {
		puts("Operation has been successful");
		memcpy((void*) dst, src, n);
		return 0;
	}

	else
		puts("Operation failed");
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
uint32_t muse_map(char *path, size_t length, int flags) {
	return 0;
}

int muse_sync(uint32_t addr, size_t len) {
	return 0;
}

int muse_unmap(uint32_t dir) {
	return 0;
}
