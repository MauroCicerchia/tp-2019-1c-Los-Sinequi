#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<commons/log.h>

#include "consistency.h"

char **validate_query_and_return_args(char*);
int sizeofArray(char**);
void start_API(t_log *);
