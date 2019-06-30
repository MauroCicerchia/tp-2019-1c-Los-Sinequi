#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include"consistency.h"
#include"console.h"
#include<errno.h>

#ifndef QUERY_H
#define QUERY_H


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
	t_list *args;
} t_query;

t_query *query_create(e_query, t_list*);
void query_destroy(void*);
e_query getQueryType(char*);
e_query queryError();
int validateQuerySyntax(t_list*);
t_list *parseQuery(char*);
int isNumeric(char*);

#endif
