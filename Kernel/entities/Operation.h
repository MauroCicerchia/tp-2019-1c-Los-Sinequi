#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<sys/time.h>
#include<stdbool.h>

#ifndef OPERATION_H
#define OPERATION_H

typedef enum {
	OP_READ,
	OP_WRITE
} e_op;

typedef struct {
	e_op type;
	uint64_t timestamp;
	uint64_t duration;
} operation_t;

operation_t *new_operation(e_op, uint64_t);
void operation_destroy(void*);
bool op_is_recent(void*);

#endif
