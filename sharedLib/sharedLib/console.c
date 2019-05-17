#include"console.h"

char **validate_query_and_return_args(char *query) {
	char **args = string_split(query, " "); //guardas en el vecor args la query

	int validQuery = validateQuerySyntax(args, args[0]); //validamos que sea correcta y sino lanzamos exception
	if (!validQuery) {
		return NULL;
	}

	return args;
}

int sizeofArray(char **array){
	int i=0;
	if(!array[i]) return 0;
	while(array[i] != NULL){
		i++;
	}
	return i;
}

void start_API(t_log *logger){
	char *input;
	input = readline(">");
	while(strcmp("", input)) {
		newQuery(input, logger);
		free(input);
		input = readline(">");
	}
}
