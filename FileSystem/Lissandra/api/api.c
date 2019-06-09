#include"api.h"

int retard;

void *start_Api()
{
	char *input;
	input = readline(">");

	while(strcmp("", input)) {
		processQuery(input);
		free(input);
		input = readline("\n>");
	}

	free(input);
	return NULL;
}

void processQuery(char *query)
{
	e_query queryType;
	char *selectReturnValue = string_new();
	char **args = parseQuery(query);

	queryType = getQueryType(args[0]); //guardamos el tipo de query por ej: SELECT

	int invalidQuery = validateQuerySyntax(args, queryType); //validamos que sea correcta y sino lanzamos exception
	if (!invalidQuery){
		log_error(logger,"Query invalida");
		return;
	}

	switch(queryType) { //identificamos query y procedemos a su ejecucion

		case QUERY_SELECT:
			log_info(logger, "----------------------------------------");
			log_info(logger, "Recibi un SELECT");

			delayer();

			selectReturnValue = qselect(args[1], args[2]);

			log_info(logger, ">>>>");
			log_info(logger, selectReturnValue);
			log_info(logger, ">>>>");

			log_info(logger, "Fin SELECT");
			log_info(logger, "----------------------------------------");

			if(selectReturnValue != NULL) free(selectReturnValue);
			free(args);
			break;

		case QUERY_INSERT:
			log_info(logger, "----------------------------------------");
			log_info(logger, "Recibi un INSERT");

			delayer();

			if(args[4] == NULL) args[4] = string_itoa(getCurrentTime());

			qinsert(args[1], args[2], args[3], args[4]);

			log_info(logger, "Fin INSERT");
			log_info(logger, "----------------------------------------");

//			free(args);
			break;

		case QUERY_CREATE:
			log_info(logger, "----------------------------------------");
			log_info(logger, "Recibi un CREATE");

			delayer();

			if(qcreate(args[1], args[2], args[3], args[4])){
				log_info(logger, ">>>");
				log_info(logger, "Tabla creada con exito");
				log_info(logger, ">>>");
			}else log_error(logger,"error en la creacion");

			log_info(logger, "Fin CREATE");
			log_info(logger, "----------------------------------------");

			free(args);
			break;

		case QUERY_DESCRIBE:
			log_info(logger, "----------------------------------------");
			log_info(logger, "Recibi un DESCRIBE");

			delayer();

			//describe(args[1]);

			log_info(logger, "Fin DESCRIBE");
			log_info(logger, "----------------------------------------");

			free(args);
			break;

		case QUERY_DROP:
			log_info(logger, "----------------------------------------");
			log_info(logger, "Recibi un DROP");

			delayer();

			//drop(args[1]);

			log_info(logger, "Fin DROP");
			log_info(logger, "----------------------------------------");

			free(args);
			break;

		default:
			free(args);
	}
}


void delayer()
{
	sem_wait(&MUTEX_RETARDTIME);
	int rt = retardTime;
	sem_post(&MUTEX_RETARDTIME);
	sleep(rt/1000);
}


uint64_t getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);

	return (uint64_t)((tv.tv_sec)*1000 + (tv.tv_usec)/1000);
}



