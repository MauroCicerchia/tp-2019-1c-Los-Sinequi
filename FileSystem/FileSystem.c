#include "FileSystem.h"
t_list *memtable;
char *absoluto;
t_log *logger;
int main(int argc, char **argv) {
	memtable = list_create();
	absoluto = string_new();
	string_append(&absoluto,"/home/utnso/workspace/tp-2019-1c-Los-Sinequi/FileSystem/mnt/");
	logger = NULL;
	iniciar_logger(&logger);
//	iniciar_servidor(logger);
	start_API();

	log_destroy(logger);

	return 0;
}
