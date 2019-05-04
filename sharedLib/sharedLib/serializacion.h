#include"enums.h"
#include<stdio.h>

typedef struct{
	int size;
	void *stream;
}t_buffer;

typedef struct{
	e_query op_code;
	t_buffer *buffer;

}t_package;
