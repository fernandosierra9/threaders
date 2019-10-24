#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "hilolay_alumnos.h"

/* Lib implementation: It'll only schedule the last thread that was created */
int max_tid = 0;

char *ip_suse = "127.0.0.1";
int puerto_suse = 5003;
int suse_fd;

int suse_create(int tid){
	if (tid > max_tid) max_tid = tid;

	printf("HELLO SUSE");
	suse_fd = socket_connect_to_server(ip_suse, puerto_suse);

	if (suse_fd < 0)
	{
		socket_close_conection(suse_fd);
		return -1;
	}
	puts("Conexion con Suse establecida");

	t_newthread *newthread_send = malloc(sizeof(t_newthread));
	newthread_send->pid = 200;
	newthread_send->tid = 123;
	t_protocol newthread_protocol = NEW_THREAD;

	utils_serialize_and_send(suse_fd, newthread_protocol, newthread_send);
	utils_serialize_and_send(suse_fd, newthread_protocol, newthread_send);

	return 0;

}

int suse_schedule_next(void){
	int next = max_tid;
	printf("Scheduling next item %i...\n", next);
	return next;
}

int suse_join(int tid){
	// Not supported
	return 0;
}

int suse_close(int tid){
	printf("Closed thread %i\n", tid);
	max_tid--;
	return 0;
}

int suse_wait(int tid, char *sem_name){
	// Not supported
	return 0;
}

int suse_signal(int tid, char *sem_name){
	// Not supported
	return 0;
}

static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close,
		.suse_wait = &suse_wait,
		.suse_signal = &suse_signal
};

void hilolay_init(void){
	init_internal(&hiloops);
}
