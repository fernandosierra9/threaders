#ifndef COMMON_PROTOCOLS_H_
#define COMMON_PROTOCOLS_H_

typedef enum
{
	MESSAGE,
	PACKAGE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code operation_code;
	t_buffer* buffer;
} t_package;

#endif /* COMMON_PROTOCOLS_H_ */
