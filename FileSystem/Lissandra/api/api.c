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
	char *timeStamp;
	char log_msg[200];
	char *selectReturnValue;
	t_list *args = parseQuery(query);
	t_list *tables;

	queryType = getQueryType(list_get(args,0)); //guardamos el tipo de query por ej: SELECT

	int invalidQuery = validateQuerySyntax(args); //validamos que sea correcta y sino lanzamos exception
	if (!invalidQuery){
		log_error(logger,"Query invalida");
		return;
	}

	switch(queryType) { //identificamos query y procedemos a su ejecucion

		case QUERY_SELECT:
			sprintf(log_msg, "[API]: Recibi un SELECT de %s, Key: %s", (char*)list_get(args,1),(char*)list_get(args,2));
			log_info(logger, log_msg);

			selectReturnValue = qselect(list_get(args,1), list_get(args,2));

			if(selectReturnValue != NULL){
			sprintf(log_msg,"[API]: El valor encontrado es: %s",selectReturnValue);
			log_info(logger, log_msg);

			free(selectReturnValue);
			}

			log_info(logger, "[API]: Fin SELECT");

			break;

		case QUERY_INSERT:
			sprintf(log_msg,"[API]: Recibi un INSERT: %s Key: %s, Value: %s",(char*)list_get(args,1),(char*)list_get(args,2),(char*)list_get(args,3));
			log_info(logger, log_msg);

			timeStamp = string_from_format("%llu",getCurrentTime());
			if(list_get(args,4) == NULL) list_add(args,timeStamp);

			qinsert(list_get(args,1), list_get(args,2), list_get(args,3), list_get(args,4));

			log_info(logger,"[API]: Fin INSERT");
			break;

		case QUERY_CREATE:
			sprintf(log_msg,"[API]: Recibi un CREATE de tabla: %s",(char*)list_get(args,1));
			log_info(logger,log_msg);
			sprintf(log_msg,"Consistencia: %s, Particiones: %s, T. Compactacion: %s",(char*)list_get(args,2),(char*)list_get(args,3),(char*)list_get(args,5));
			log_info(logger,log_msg);

			if(qcreate((char*)list_get(args,1), (char*)list_get(args,2), (char*)list_get(args,3), (char*)list_get(args,4))){
				log_info(logger, "[API]: Tabla creada con exito");
			}else log_error(logger,"[API]: Error en la creacion");

			log_info(logger, "[API]: Fin CREATE");

			break;

		case QUERY_DESCRIBE:

			if(list_get(args,1) == NULL){
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
				sprintf(log_msg, "[API]: Recibi un DESCRIBE de: %s",(char*)list_get(args,1));
				log_info(logger, log_msg);

				metadata *tableInfo = qdescribe(list_get(args,1));

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


			break;

		case QUERY_DROP:
			sprintf(log_msg, "[API]: Recibi un DROP %s", (char*)list_get(args,1));
			log_info(logger,log_msg);


			qdrop(list_get(args,1));

			log_info(logger, "[API]: Fin DROP");

			break;

		default:
			break;
	}
	list_destroy_and_destroy_elements(args, free);
}

uint64_t getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	uint64_t  x = (uint64_t)( (tv.tv_sec)*1000 + (tv.tv_usec)/1000 );
	return x;
}
