#include "FileSystem.h"
t_list *memtable;

int main(int argc, char **argv) {
	memtable = list_create();
	t_log *logger = NULL;

	iniciar_logger(&logger);
//	iniciar_servidor(logger);
	start_API(logger);

	log_destroy(logger);

	return 0;
}
