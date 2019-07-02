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
	char log_msg[100];
	char *selectReturnValue;
	char **args = parseQuery(query);
	t_list *tables;

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

			if(selectReturnValue != NULL){
			log_info(logger, ">>>>");
			log_info(logger, selectReturnValue);
			log_info(logger, ">>>>");

			free(selectReturnValue);
			}

			log_info(logger, "Fin SELECT");
			log_info(logger, "----------------------------------------");

			free(args);
			break;

		case QUERY_INSERT:
			log_info(logger, "----------------------------------------");
			log_info(logger, "Recibi un INSERT");

			delayer();

			if(args[4] == NULL) args[4] = string_from_format("%llu",getCurrentTime());

			qinsert(args[1], args[2], args[3], args[4]);

			log_info(logger, "Fin INSERT");
			log_info(logger, "----------------------------------------");

//			free(args[1]); free(args[2]); free(args[3]); free(args[4]);
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

			if(args[1] == NULL){
				tables = fs_getAllTables();
				for(int i = 0; i < list_size(tables); i++){
					metadata *tableInfo = qdescribe(list_get(tables,i));

					if(tableInfo != NULL){
						log_info(logger, ">>>");
						log_info(logger,"TABLA:");
						log_info(logger,list_get(tables,i));
						sprintf(log_msg,"Consistencia: %s",tableInfo->consistency);
						log_info(logger,log_msg);
						sprintf(log_msg,"Particiones: %s",tableInfo->partitions);
						log_info(logger,log_msg);
						sprintf(log_msg,"Tiempo de compactacion: %s",tableInfo->ctime);
						log_info(logger,log_msg);
						log_info(logger, ">>>");

						free(tableInfo->consistency); free(tableInfo->ctime); free(tableInfo->partitions);
						free(tableInfo);
//						free(cons);
					}
				}
			}else{
				metadata *tableInfo = qdescribe(args[1]);

				if(tableInfo != NULL){
					log_info(logger, ">>>");
					char *cons = string_new(); strcpy(cons,tableInfo->consistency);
					char *parts = tableInfo->partitions; char *ctime = tableInfo->ctime;
					sprintf(log_msg,"Consistencia: %s",cons);
					log_info(logger,log_msg);
					sprintf(log_msg,"Particiones: %s",parts);
					log_info(logger,log_msg);
					sprintf(log_msg,"Tiempo de compactacion: %s",ctime);
					log_info(logger,log_msg);
					log_info(logger, ">>>");

					free(tableInfo->consistency); free(tableInfo->ctime); free(tableInfo->partitions);
					free(tableInfo);
					free(cons);
				}
			}
			log_info(logger, "Fin DESCRIBE");
			log_info(logger, "----------------------------------------");

			free(args);

			break;

		case QUERY_DROP:
			log_info(logger, "----------------------------------------");
			log_info(logger, "Recibi un DROP");

			delayer();

			qdrop(args[1]);

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
	uint64_t  x = (uint64_t)( (tv.tv_sec)*1000 + (tv.tv_usec)/1000 );
	return x;
}
