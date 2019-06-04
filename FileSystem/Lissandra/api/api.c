#include"api.h"

void start_API(t_log *logger){

	char *input;
	input = readline(">");
	while(strcmp("", input)) {
		processQuery(input, logger);
		free(input);
		input = readline(">");
	}
	free(input);
}

e_query processQuery(char *query, t_log *logger) {

	char log_msg[100];
	e_query queryType;
	char **args = parseQuery(query);

	queryType = getQueryType(args[0]); //guardamos el tipo de query por ej: SELECT

	int invalidQuery = validateQuerySyntax(args, queryType); //validamos que sea correcta y sino lanzamos exception
	if (!invalidQuery){
		return queryError();
	}

	switch(queryType) { //identificamos query y procedemos a su ejecucion

		case QUERY_SELECT:

			printf("%s",qselect(args[1], args[2]));
//			sprintf(log_msg, "Recibi un SELECT %s %s", args[1], args[2]);

			break;

		case QUERY_INSERT:

			if(args[4] == NULL) args[4] = string_itoa(getCurrentTime());
			qinsert(args[1], args[2], args[3], args[4]);
//			sprintf(log_msg, "Recibi un INSERT %s %s %s", args[1], args[2], args[3]);

			break;

		case QUERY_CREATE:
			qcreate(args[1], args[2], args[3], args[4]);
			printf("creado con exito\n");
//			sprintf(log_msg, "Recibi un CREATE %s %s %s %s", args[1], args[2], args[3], args[4]);
			break;

		case QUERY_DESCRIBE:

			//describe(args[1]);

			sprintf(log_msg, "Recibi un DESCRIBE %s", args[1]);

			break;

		case QUERY_DROP:

			//drop(args[1]);

			sprintf(log_msg, "Recibi un DROP %s", args[1]);

			break;

		default:
			return queryError(logger);

	}

//	log_info(logger, log_msg);
	return queryType;
}


uint64_t getCurrentTime(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (uint64_t)((tv.tv_sec)*1000 + (tv.tv_usec)/1000);
}

char **parseQuery(char *query){
	char **args = string_split(query, " ");
	if(args[1] == NULL){
		char **insert = string_split(query, "\"");
		char **pivot = string_split(insert[0], " ");
		strcpy(pivot[3],insert[1]);
		strcpy(pivot[4],insert[2]);
		free(insert);
		return pivot;
	}
	return args;
}
