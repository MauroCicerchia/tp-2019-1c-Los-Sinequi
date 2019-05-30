#include "Memory.h"
int server;

int main(int argc, char **argv) {
	segmentList = list_create();
	/*
	t_log *logger = NULL;
	char *input;
	//server = conectar_FS(logger);
	iniciar_logger(&logger);
	pthread_t threadKernel;
	pthread_t threadFS;

	//2 hilos diferentes
//	pthread_create(&threadFS,NULL,start_API,logger);
	//pthread_create(&threadKernel,NULL,conectar_Kernel,logger);


	//pthread_join(threadKernel,NULL);
	pthread_join(threadFS,NULL);
	closeConnection(server);

//	conectar_Kernel(logger);//conectar con kernel
//	conectar_FS(logger);
//	start_API(logger);

	log_destroy(logger);
	//segment testSegment;

	//testSegment=segment_init();

*/
	return 0;
}

page* search_page(segment* aSegment,int aKey){
	bool isKey(void* aPage){
			return ((page*) aPage)->page_data->key == aKey;
		}
		return list_find(aSegment->page_list,isKey);
}

segment* search_segment(char* segmentID){
	//proximamente buscar en tabla de segmentos
	bool isId(void* aSegment){
		return strcasecmp(((segment*) aSegment)->segment_id,segmentID)==0;
	}
	return list_find(segmentList,isId);
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
			sendMessage(server,query);
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
