#include"enums.h"
#include<stdio.h>
#include<stdlib.h>

typedef struct{
	int size;
	void *stream;
}t_buffer;

typedef struct{
	e_query op_code;
	t_buffer *buffer;

}t_package;

void *serialize_package(t_package*, int);
t_package *create_package(e_query);
void create_buffer(t_package*);
void add_to_package(t_package*, void*, int);
void send_package(t_package*, int);
void delete_package(t_package*);
void set_buffer(t_package*, char**);
