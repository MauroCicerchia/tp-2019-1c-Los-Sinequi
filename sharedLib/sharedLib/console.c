#include"console.h"

char **validate_query_and_return_args(char *query) {
	char **args = parseQuery(query); //guardas en el vecor args la query

	int validQuery = validateQuerySyntax(args, getQueryType(args[0])); //validamos que sea correcta y sino lanzamos exception
	if (!validQuery) {
		free(args);
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
	input = (char*)readline(">");
	while(strcmp("", input)) {
		processQuery(input, logger);
		add_history(input);
		free(input);
		printf("\033[A\33[2K\r");
		input = readline(">");
	}
	free(input);
}
