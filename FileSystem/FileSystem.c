#include "FileSystem.h"

typedef struct{
	int timeStamp;
	char *key;
	char *value;
}data;



int main(int argc, char **argv) {

	t_log *logger = NULL;

	iniciar_logger(&logger);
//	iniciar_servidor(logger);
	start_API(logger);

	log_destroy(logger);

	return 0;
}

char* processQuery(char *query, t_log *logger) {

	char log_msg[100];
	e_query queryType;

	char **args = string_split(query, " "); //guardas en el vecor args la query

	queryType = getQueryType(args[0]); //guardamos el tipo de query por ej: SELECT

	int invalidQuery = validateQuerySyntax(args, queryType); //validamos que sea correcta y sino lanzamos exception
	if (!invalidQuery){
		return queryError();
	}

	switch(queryType) { //identificamos query y procedemos a su ejecucion

		case QUERY_SELECT:

			qselect(args[1], args[2]);

			sprintf(log_msg, "Recibi un SELECT %s %s", args[1], args[2]);

			break;

		case QUERY_INSERT:

			qinsert(args[1], args[2], args[3], args[4]);

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

		default:
			return queryError(logger);

	}

	log_info(logger, log_msg);
	return queryType;
}

void qinsert(char *table, int key, char *value, int timeStamp){

	char *url = string_append("tables/", table);
	url = string_append(url, ".bin");
	FILE *file = txt_open_for_append(url);
	free(url);
	char *insert = toLFSmode(timeStamp,key,value);
	txt_write_in_file(file, insert);
	free(insert);
	txt_close_file(file);

}

char *qselect(char *table, int key){

	char *url = string_append("tables/", table);
	url = string_append(url, ".bin");
	FILE *file = fopen(url,"r");

	t_list *list = list_create();

	loadList(list,file);
	txt_close_file(file);
	toDATAmode(list);
	char *value = getValue(list,key);

	list_destroy(list);
	return value;
}

void loadList(t_list *list,FILE file){

}

char *getValue(t_list list,int key){

}

data *toDATAmode(t_list *list){

}

char *toLFSmode(int timeStamp, int key, char *value){

	char *insert = string_append(timeStamp, ";");
	char *sKey = string_itoa(key);
	insert = string_append(insert, sKey);
	free(sKey);
	insert = string_append(insert, ";");
	insert = string_append(insert, value);
	insert = string_append(insert, "\n");
	return insert;
}

//si un puntero apunta a tal direccion de memoria(0x1) y cambio a donde apunta (0x2), la antigua posicion(0x1) entiendo que la perdi, pero.. queda libre para que se use? o hay que hacerle free?

/*
 * 	char *value = getValue(list,key);
 *
 *	list_destroy(list);
 *	return value;
 *
 *	como hago el free de value?
*/

