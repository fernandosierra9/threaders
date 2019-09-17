/*
 ============================================================================
 Name        : muse.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	int response = muse_logger_create();
	if (response < 0)
		return response;

	response = muse_config_load();
	if (response < 0)

	{
		muse_logger_destroy();
		return response;
	}
	muse_config_free();
	muse_logger_destroy();
	return EXIT_SUCCESS;
}
