#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<commons/log.h>

#ifndef CONSOLE_H
#define CONSOLE_H

char **validate_query_and_return_args(char*);
int sizeofArray(char**);
void start_API(t_log *);

#endif
