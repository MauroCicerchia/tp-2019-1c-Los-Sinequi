#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<commons/log.h>
#include<commons/collections/list.h>

#ifndef CONSOLE_H
#define CONSOLE_H

t_list *validate_query_and_return_args(char*);
int sizeofArray(char**);
void start_API(t_log *);
void string_destroy(void*);
void string_destroy_char(char*);

#endif
