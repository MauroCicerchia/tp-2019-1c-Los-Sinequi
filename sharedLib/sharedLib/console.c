#include"console.h"

t_list *validate_query_and_return_args(char *query) {
	t_list *args = parseQuery(query); //guardas en la lista args la query

	int validQuery = validateQuerySyntax(args); //validamos que sea correcta y sino lanzamos exception
	if (!validQuery) {
		list_destroy_and_destroy_elements(args, string_destroy);
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

void start_API(t_log *logger) {
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

void string_destroy(void *str) {
	free(str);
}
void string_destroy_char(char* str){
	free(str);
}
