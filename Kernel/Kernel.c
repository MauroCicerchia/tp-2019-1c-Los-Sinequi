#include "Kernel.h"


int server;

int main(int argc, char **argv) {

	t_log *logger = NULL;

	server = iniciar_cliente();
	iniciar_logger(&logger);

	start_API(logger);

	closeConnection(server);

	log_destroy(logger);

	return 0;
}

e_query processQuery(char *query, t_log *logger) {

	char log_msg[100];
	e_query queryType;

	char **args = string_split(query, " "); //guardas en el vecor args la query

	queryType = getQueryType(args[0]); //guardamos el tipo de query por ej: SELECT

	int invalidQuery = validateQuerySyntax(args, queryType); //validamos que sea correcta y sino lanzamos exception
	if (!invalidQuery){
		return queryError();
	}

	switch(queryType) {

		case QUERY_SELECT:

			//select(args[1], args[2]);
			sendMessage(server,query);
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

		case QUERY_ADD:

			//add(args[2], args[4]);

			sprintf(log_msg, "Recibi un ADD MEMORY %s TO %s", args[2], args[4]);

			break;

		case QUERY_RUN:

			//run(args[1]);

			sprintf(log_msg, "Recibi un RUN %s", args[1]);

			break;

		case QUERY_METRICS:

			//metrics();

			sprintf(log_msg, "Recibi un METRICS");

			break;

		default:
			return queryError(logger);

	}

	log_info(logger, log_msg);
	return queryType;
}
