#include "FileSystem.h"

int main(int argc, char **argv) {

	t_log *logger = NULL;

	iniciar_logger(&logger);
//	iniciar_servidor(logger);
	start_API(logger);

	log_destroy(logger);

	return 0;
}
