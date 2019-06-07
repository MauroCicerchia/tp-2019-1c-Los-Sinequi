#include"query.h"

t_query *query_create(e_query queryType, char **args) {
	t_query *newQuery = (t_query*) malloc(sizeof(t_query));
	newQuery->queryType = queryType;
	newQuery->args = args;
	return newQuery;
}

void query_destroy(void *query) {
	void string_destroy(char *str) {
		free(str);
	}
	string_iterate_lines(((t_query*)query)->args, string_destroy);
	free(query);
}

e_query getQueryType(char *query) {
	if (!strcasecmp("SELECT", query))
		return QUERY_SELECT;
	else if (!strcasecmp("INSERT", query))
		return QUERY_INSERT;
	else if (!strcasecmp("CREATE", query))
			return QUERY_CREATE;
	else if (!strcasecmp("DESCRIBE", query))
			return QUERY_DESCRIBE;
	else if (!strcasecmp("DROP", query))
			return QUERY_DROP;
	else if (!strcasecmp("JOURNAL", query))
			return QUERY_JOURNAL;
	else if (!strcasecmp("ADD", query))
			return QUERY_ADD;
	else if (!strcasecmp("RUN", query))
			return QUERY_RUN;
	else if (!strcasecmp("METRICS", query))
			return QUERY_METRICS;
	return QUERY_ERROR;
}

e_query queryError() {
	printf("La consulta no es valida.\n");
	return QUERY_ERROR;
}

int validateQuerySyntax(char **array,e_query queryType){
	int tamano = sizeofArray(array);
	int key;
	switch(queryType) {

		case QUERY_SELECT:
			if( tamano != 3) return false; // cantidad de parametros invalidos
			key = atoi(array[2]);
			if(!key) return false; //key invalida
			return true;

		case QUERY_INSERT:
			if( tamano != 5) return false; // cantidad de parametros invalidos
			key = atoi(array[2]);
			if(!key) return false; //key invalida
			return true;

		case QUERY_CREATE:
			if( sizeofArray(array) != 5 ) return 0; // cantidad de parametros invalidos

			key = atoi(array[3]);
			if(!key) return false; //particiones o tiempo de compactacion invalidos

			key = atoi(array[4]);
			if(!key) return false;

			return true;

		case QUERY_DESCRIBE:
			if( sizeofArray(array) != 2 ) return false; // cantidad de parametros invalidos
			return true;

		case QUERY_DROP:
			if( sizeofArray(array) != 2 ) return false; // cantidad de parametros invalidos
			return true;

		case QUERY_JOURNAL:
			if( sizeofArray(array) != 1 ) return false; // cantidad de parametros invalidos
			return true;

		case QUERY_ADD:
			if( sizeofArray(array) != 5 ) return false; // cantidad de parametros invalidos
			if(strcasecmp(array[1], "MEMORY") != 0) return false;
			if(strcasecmp(array[3], "MEMORY") != 0) return false;
			if(!atoi(array[2])) return false;
			if(getConsistencyType(array[4]) == CONS_ERROR) return false;
			return true;

		case QUERY_RUN:
			if( sizeofArray(array) != 2 ) return false; // cantidad de parametros invalidos
			return true;

		default:
			return true;
	}
}

char **parseQuery(char *query){
	if(string_starts_with(query,"INSERT")){
		char **a = string_split(query,"\"");
		char **b = string_split(a[0]," ");
		b[3] = strdup(a[1]);
		string_trim(&a[2]);
		b[4] = strdup(a[2]);
		b[5] = NULL;
		return b;
	}
	char **c = string_split(query," ");
	return c;
}
