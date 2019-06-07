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
			if( tamano != 3) return 0; // cantidad de parametros invalidos
			if(!isNumeric(array[2])) return 0;
			key = strtol(array[2], NULL, 10);
			if(errno == EINVAL || errno == ERANGE) return 0; //key invalida
			return 1;
			break;

		case QUERY_INSERT:
			if( sizeofArray(array) != 5 && sizeofArray(array) != 4) return 0; // cantidad de parametros invalidos
			if(!isNumeric(array[2])) return 0;
			key = strtol(array[2], NULL, 10);
			if(!key && (errno == EINVAL || errno == ERANGE)) return 0; //key invalida
			return 1;
			break;

		case QUERY_CREATE:
			if( sizeofArray(array) != 5 ) return 0; // cantidad de parametros invalidos

			if(!isNumeric(array[3])) return 0;
			key = strtol(array[3], NULL, 10);
			if(!key && (errno == EINVAL || errno == ERANGE)) return 0; //particiones o tiempo de compactacion invalidos

			if(!isNumeric(array[4])) return 0;
			key = strtol(array[4], NULL, 10);
			if(!key && (errno == EINVAL || errno == ERANGE)) return 0;

			return 1;
			break;

		case QUERY_DESCRIBE:
			if( sizeofArray(array) != 2 ) return 0; // cantidad de parametros invalidos
			return 1;
			break;

		case QUERY_DROP:
			if( sizeofArray(array) != 2 ) return 0; // cantidad de parametros invalidos
			return 1;
			break;

		case QUERY_JOURNAL:
			if( sizeofArray(array) != 1 ) return 0; // cantidad de parametros invalidos
			return 1;
			break;

		case QUERY_ADD:
			if( sizeofArray(array) != 5 ) return 0; // cantidad de parametros invalidos
			if(strcasecmp(array[1], "MEMORY") != 0) return 0;
			if(strcasecmp(array[3], "TO") != 0) return 0;
			if(!isNumeric(array[2])) return 0;
			key = strtol(array[2], NULL, 10);
			if(!key && (errno == EINVAL || errno == ERANGE)) return 0;
			if(getConsistencyType(array[4]) == CONS_ERROR) return 0;
			return 1;
			break;

		case QUERY_RUN:
			if( sizeofArray(array) != 2 ) return 0; // cantidad de parametros invalidos
			return 1;
			break;

		default:
			return 0;
			break;
	}
}

char **parseQuery(char *query){
//	[INSERT T K "V" T]
//	[INSERT T K] [V] [T]
//	[INSERT] [T] [K]      [V] [T]

	char **b;

	if(string_starts_with(query,"INSERT")) {
		char **a = string_split(query,"\"");
		b = string_split(a[0]," ");
		b[3] = string_duplicate(a[1]);

		if(!(string_ends_with(query, "\"") || !!string_ends_with(query, "\"\n"))) {
			string_trim(&a[2]);
			b[4] = string_duplicate(a[2]);
			b[5] = NULL;
		} else {
			b[4] = NULL;
		}
	} else {
		b = string_split(query," ");
	}
	return b;
}

int isNumeric(char *str) {
	int i = 0;
	while(str[i] != '\0' && str[i] != '\n') {
		if(!isdigit(str[i]))
			return 0;
		i++;
	}
	return 1;
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
