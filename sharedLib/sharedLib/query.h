#include<stdlib.h>
#include<commons/string.h>

typedef enum {
	QUERY_SELECT,
	QUERY_INSERT,
	QUERY_CREATE,
	QUERY_DESCRIBE,
	QUERY_DROP,
	QUERY_JOURNAL,
	QUERY_ADD,
	QUERY_RUN,
	QUERY_METRICS,
	QUERY_ERROR
}e_query;

typedef struct {
	e_query queryType;
	char **args;
} t_query;

t_query *query_create(e_query, char**);
void query_destroy(void*);
e_query getQueryType(char*);
e_query queryError();
int validateQuerySyntax(char**,e_query);
