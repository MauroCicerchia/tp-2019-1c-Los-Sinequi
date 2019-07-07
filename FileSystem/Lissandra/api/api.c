#include"api.h"

int retard;

void *start_Api()
{
	char *input;
	input = readline(">");

	while(strcmp("", input)) {
		processQuery(input);
		add_history(input);
		free(input);
		input = readline("\n>");
	}

	free(input);
	return NULL;
}

void processQuery(char *query)
{
	e_query queryType;
	char log_msg[200];
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
			sprintf(log_msg, "[API]: Recibi un SELECT de %s, Key: %s",args[1],args[2]);
			log_info(logger, log_msg);

			selectReturnValue = qselect(args[1], args[2]);

			if(selectReturnValue != NULL){
			sprintf(log_msg,"[API]: El valor encontrado es: %s",selectReturnValue);
			log_info(logger, log_msg);

			free(selectReturnValue);
			}

			log_info(logger, "[API]: Fin SELECT");

			free(args);
			break;

		case QUERY_INSERT:
			sprintf(log_msg,"[API]: Recibi un INSERT: %s Key: %s, Value: %s",args[1],args[2],args[3]);
			log_info(logger, log_msg);

			if(args[4] == NULL) args[4] = string_from_format("%llu",getCurrentTime());

			qinsert(args[1], args[2], args[3], args[4]);

			log_info(logger,"[API]: Fin INSERT");
//			free(args[1]); free(args[2]); free(args[3]); free(args[4]);
			break;

		case QUERY_CREATE:
			sprintf(log_msg,"[API]: Recibi un CREATE de tabla: %s",args[1]);
			log_info(logger,log_msg);
			sprintf(log_msg,"Consistencia: %s, Particiones: %s, T. Compactacion: %s",args[2],args[3],args[5]);
			log_info(logger,log_msg);

			if(qcreate(args[1], args[2], args[3], args[4])){
				log_info(logger, "[API]: Tabla creada con exito");
			}else log_error(logger,"[API]: Error en la creacion");

			log_info(logger, "[API]: Fin CREATE");

			free(args);
			break;

		case QUERY_DESCRIBE:

			if(args[1] == NULL){
				log_info(logger,"[API]: Recibi un DESCRIBE global");
				tables = fs_getAllTables();
				for(int i = 0; i < list_size(tables); i++){
					metadata *tableInfo = qdescribe(list_get(tables,i));

					if(tableInfo != NULL){

						sprintf(log_msg,"[API]: Tabla: %s -> Cons: %s, Parts: %s, T.Comp: %s",(char*)list_get(tables,i),tableInfo->consistency,tableInfo->partitions,tableInfo->ctime);
						log_info(logger,log_msg);

						free(tableInfo->consistency); free(tableInfo->ctime); free(tableInfo->partitions);
						free(tableInfo);
//						free(cons);
					}
				}
				list_clean_and_destroy_elements(tables, free);
			}else{
				sprintf(log_msg, "[API]: Recibi un DESCRIBE de: %s",args[1]);
				log_info(logger, log_msg);

				metadata *tableInfo = qdescribe(args[1]);

				if(tableInfo != NULL){
					char *cons = string_duplicate(tableInfo->consistency);
					char *parts = tableInfo->partitions; char *ctime = tableInfo->ctime;

					sprintf(log_msg,"[API]: Consistencia: %s, Particiones: %s, T. Compactacion: %s",cons,parts,ctime);
					log_info(logger,log_msg);

					free(tableInfo->consistency); free(tableInfo->ctime); free(tableInfo->partitions);
					free(tableInfo);
					free(cons);
				}
			}
			log_info(logger, "[API]: Fin DESCRIBE");

			free(args[1]);
//			free(args);

			break;

		case QUERY_DROP:
			sprintf(log_msg, "[API]: Recibi un DROP %s", args[1]);
			log_info(logger,log_msg);


			qdrop(args[1]);

			log_info(logger, "[API]: Fin DROP");

			free(args[1]);
			break;

		default:
			free(args);
			break;
	}

}

uint64_t getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	uint64_t  x = (uint64_t)( (tv.tv_sec)*1000 + (tv.tv_usec)/1000 );
	return x;
}
