#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include"enums.h"

e_query getQueryType(char*);
e_query queryError();
int validateQuerySyntax(char**,e_query);
int sizeofArray(char**);
