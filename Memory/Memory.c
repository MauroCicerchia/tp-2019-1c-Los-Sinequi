#include "Memory.h"

int main(int argc, char **argv) {

	t_log *logger = NULL;
	char *input;

	iniciar_logger(&logger);

	//2 hilos diferentes
//	conectar_Kernel(logger);//conectar con kernel
	conectar_FS(logger);


//	input = readline(">");
//
//	while(strcmp("", input)) {
//
//		processQuery(input, logger);
//		free(input);
//		input = readline(">");
//
//	}

	log_destroy(logger);

	return 0;
}

/*void processQueryList(t_list *querys, t_log *logger) {
	void processQueryWithLogger(void *query) {
		processQuery((char *)query, logger);
		//printf("%s", (char*)query);
	}
	list_iterate(querys, processQueryWithLogger);
}*/

e_query processQuery(char *query, t_log *logger) {

	char log_msg[100];
	e_query queryType;

	char **args = string_split(query, " ");

	queryType = getQueryType(args[0]);

	switch(queryType) {

		case QUERY_SELECT:

			//select(args[1], args[2]);
//			queryToFileSystem(*query);
			sprintf(log_msg, "Recibi un SELECT %s %s", args[1], args[2]);

			break;

		case QUERY_INSERT:

			//insert(args[1], args[2], args[3], args[4]);

			sprintf(log_msg, "Recibi un INSERT %s %s %s", args[1], args[2], args[3]);

			break;

		case QUERY_CREATE:

			//create(args[1], args[2], args[3], args[4]);

			sprintf(log_msg, "Recibi un CREATE %s %s %s %s", args[1], args[2], args[3], args[4]);

			break;

		case QUERY_DESCRIBE:

			//describe(args[1]);

			sprintf(log_msg, "Recibi un DESCRIBE %s", args[1]);

			break;

		case QUERY_DROP:

			//drop(args[1]);

			sprintf(log_msg, "Recibi un DROP %s", args[1]);

			break;

		case QUERY_JOURNAL:

			//journal();

			sprintf(log_msg, "Recibi un JOURNAL");

			break;

		default:
			return queryError(logger);

	}

	log_info(logger, log_msg);
	return queryType;
}
