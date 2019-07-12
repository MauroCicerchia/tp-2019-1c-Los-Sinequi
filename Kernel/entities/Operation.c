#include"Operation.h"

operation_t *new_operation(e_op type, uint64_t duration) {
	operation_t *new_op = (operation_t*)malloc(sizeof(operation_t));
	new_op->type = type;
	new_op->duration = duration;
	new_op->timestamp = getCurrentTime();
	return new_op;
}

void operation_destroy(void *op) {
	free(op);
}

bool op_is_recent(void *op) {
	return getCurrentTime() - ((operation_t*)op)->timestamp < 30000;
}
