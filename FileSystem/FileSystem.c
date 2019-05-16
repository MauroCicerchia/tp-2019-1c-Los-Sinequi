#include "FileSystem.h"

int main(int argc, char **argv) {

	t_log *logger = NULL;

	iniciar_logger(&logger);
//	iniciar_servidor(logger);
	start_API(logger);

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

	switch(queryType) { //identificamos query y procedemos a su ejecucion

		case QUERY_SELECT:

//			qselect(args[1], args[2]);

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

//
//void qinsert(char *table, uint16_t key, char *value, int timeStamp){
//
//	char *url =  string_new();
//	string_append(&url,"tables/");
//	string_append(&url,table);
//	string_append(&url, ".bin");
//	FILE *file = txt_open_for_append(url);
//	free(url);
//	char *toInsert = string_new();
//	toInsert = toLFSmode(timeStamp,key,value);
//	txt_write_in_file(file, toInsert);
//	free(toInsert);
//	txt_close_file(file);
//}
//
////recibe por parametro timestamp,key y value, los pasa a string y duelve un string de tipo "timestamp;key,value"
//char* toLFSmode( int timeStamp, uint16_t key, char *value){
//	char *insert = string_new();
//	string_append(&insert,string_itoa(timeStamp));
//	string_append(&insert, ";");
//	char *sKey = string_itoa(key);
//	string_append(&insert, sKey);
//	free(sKey);
//	string_append(&insert, ";");
//	string_append(&insert, value);
//	string_append(&insert, "\n");
//	return insert;
//
//}
//
//
//char *qselect(char *table, uint16_t key){
//
//	char *url = string_new();
//	string_append(&url,"tables/");
//	string_append(&url,table);
//	string_append(&url, ".bin");
//	FILE *file = fopen(url,"r");
//	free(url);
//	t_list *list = list_create();
//
//	loadList(list,file);
//	txt_close_file(file);
//	t_list *dataList = listToDATAmode(list);
//	list_destroy(list);
//	char *value = string_new();
//	value = getValue(dataList,key);
//	list_destroy(dataList);
//	return value;
//}
//
////carga la lista con la info del archivo
//void loadList(t_list *list,FILE *file){
//	char *line = string_new();
//	while(!feof(file)){
//		fgets(line,100,file);
//		list_add(list,line);
//	}
//	free(line);
//}
//
////duelvue el ultimo valor de la lista que matchea con la key
//char *getValue(t_list *list,uint16_t key){
//	t_list *pivot = malloc(sizeof(t_list) * list_size(list));
//	pivot = list_sorted(list,biggerTimeStamp);
//
//	bool _lastKey(void *elem){
//		return isLastKey(key,elem);
//	}
//
//	dataSelect *returnValue =  (dataSelect*) list_find(pivot, _lastKey);
//	free(pivot);
//	return (*returnValue).value;
//}
//
////duelvue true si el priemr elemento fue agregado despues que el segundo
//bool biggerTimeStamp(dataSelect elem1, dataSelect elem2){
//	return (elem1.timeStamp > elem2.timeStamp);
//}
//
////devuelve true si el elemento que se le pasa coincide con la key
//bool isLastKey(uint16_t key,void* elem){
//	dataSelect *pivot = malloc(sizeof(dataSelect));
//	pivot =(dataSelect*) elem;
//	if(key == pivot->key){
//		free(pivot);
//		return true;
//	}
//	else {
//		free(pivot);
//		return false;
//	}
//}
//
//
////recibe una lista de "timestamp;key;value" y devuelve una transformada a tipo dataSelect
//t_list *listToDATAmode(t_list *list){
//	t_list *pivot = list_create();
//	pivot = list_map(list,elemToDATAmode);
//	return pivot;
//}
//
////recibe un elemento de tipo "timestamp;key;value" devuelve uno de tipo dataSelect con los campos asignados respectivamente
//void *elemToDATAmode(void *lfsElem){
//	char **lfsArray =string_split(lfsElem, ";");
//	dataSelect *pivot = malloc(sizeof(dataSelect));
//	int a = atoi(lfsArray[0]);
//	pivot->timeStamp = a;
//	a = atoi(lfsArray[1]);
//	pivot->key = a;
//	pivot->value = lfsArray[2];
//	return pivot;
//}
//
//
//
//
//










//si un puntero apunta a tal direccion de memoria(0x1) y cambio a donde apunta (0x2), la antigua posicion(0x1) entiendo que la perdi, pero.. queda libre para que se use? o hay que hacerle free?

/*
 * 	char *value = getValue(list,key);
 *
 *
 *	como hago el free de value? -> free de lo que retorno
 *
*/

