#include"query.h"

t_query *query_create(e_query queryType, t_list *args) {
	t_query *newQuery = (t_query*) malloc(sizeof(t_query));
	newQuery->queryType = queryType;
	newQuery->args = args;
	return newQuery;
}

void query_destroy(void *query) {
	list_destroy_and_destroy_elements(((t_query*)query)->args, string_destroy);
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
	printf("La consulta no es valida. ");
	return QUERY_ERROR;
}

int validateQuerySyntax(t_list *array){
	int size = list_size(array);
	int key;

	if(size == 0) return 0;

	e_query queryType = getQueryType(list_get(array, 0));

	switch(queryType) {

		case QUERY_SELECT:
			if(size != 3) return 0; // cantidad de parametros invalidos
			if(!isNumeric(list_get(array, 2))) return 0;
			key = strtol(list_get(array, 2), NULL, 10);
			if(errno == EINVAL || errno == ERANGE) return 0; //key invalida
			return 1;
			break;

		case QUERY_INSERT:
			if(size != 5 && size != 4) return 0; // cantidad de parametros invalidos
			if(!isNumeric(list_get(array, 2))) return 0;
			key = strtol(list_get(array, 2), NULL, 10);
			if(!key && (errno == EINVAL || errno == ERANGE)) return 0; //key invalida
			return 1;
			break;

		case QUERY_CREATE:
			if(size != 5) return 0; // cantidad de parametros invalidos

			if(getConsistencyType((char*)list_get(array,2)) == CONS_ERROR) return 0;

			if(!isNumeric(list_get(array, 3))) return 0;
			key = strtol(list_get(array, 3), NULL, 10);
			if(!key && (errno == EINVAL || errno == ERANGE)) return 0; //particiones o tiempo de compactacion invalidos

			if(!isNumeric(list_get(array, 4))) return 0;
			key = strtol(list_get(array, 4), NULL, 10);
			if(!key && (errno == EINVAL || errno == ERANGE)) return 0;

			return 1;
			break;

		case QUERY_DESCRIBE:
			if(size != 2 && size != 1) return 0; // cantidad de parametros invalidos
			return 1;
			break;

		case QUERY_DROP:
			if(size != 2) return 0; // cantidad de parametros invalidos
			return 1;
			break;

		case QUERY_JOURNAL:
			if(size != 1) return 0; // cantidad de parametros invalidos
			return 1;
			break;

		case QUERY_ADD:
			if(size != 5) return 0; // cantidad de parametros invalidos
			if(strcasecmp(list_get(array, 1), "MEMORY") != 0) return 0;
			if(strcasecmp(list_get(array, 3), "TO") != 0) return 0;
			if(!isNumeric(list_get(array, 2))) return 0;
			key = strtol(list_get(array, 2), NULL, 10);
			if(!key && (errno == EINVAL || errno == ERANGE)) return 0;
			if(getConsistencyType(list_get(array, 4)) == CONS_ERROR) return 0;
			return 1;
			break;

		case QUERY_RUN:
			if(size != 2) return 0; // cantidad de parametros invalidos
				return 1;
			break;

		case QUERY_METRICS:
			if(size != 1) return 0; // cantidad de parametros invalidos
			return 1;
			break;

		default:
			return 0;
			break;
	}
}

t_list *parseQuery(char *query){
	t_list *args = list_create();

	if(!string_starts_with(query,"RUN") && !string_starts_with(query, "run") && !string_starts_with(query, "Run"))string_to_upper(query);

	void add_to_args(char *s) {
		string_trim(&s);
//		string_to_upper(s);
		list_add(args, string_duplicate(s));
		free(s);
	}

	if(query == NULL)
		return args;

	if(string_starts_with(query, "INSERT")) {
		if(!string_contains(query, "\""))
			return args;

		char **fSplit = string_split(query, "\"");
		char **sSplit = string_split(fSplit[0], " ");
		for(int i = 0; i < sizeofArray(sSplit); i++) {
			string_trim(&sSplit[i]);
//			string_to_upper(sSplit[i]);
			list_add(args, string_duplicate(sSplit[i]));
			free(sSplit[i]);
		}
		if(fSplit[1] != NULL) {
			string_trim(&fSplit[1]);
//			string_to_upper(fSplit[1]);
			list_add(args, string_duplicate(fSplit[1]));
			free(fSplit[1]);
			if(fSplit[2] != NULL) {
				string_trim(&fSplit[2]);
//				string_to_upper(fSplit[2]);
				list_add(args, string_duplicate(fSplit[2]));
				free(fSplit[2]);
			}
		}
		free(fSplit[0]);
		free(fSplit);
		free(sSplit);
	} else {
		char **result = string_split(query, " ");
		string_iterate_lines(result, add_to_args);
		free(result);
	}
	return args;
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
