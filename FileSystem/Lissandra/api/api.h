#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<readline/readline.h>
//#include<sharedLib/console.h>
#include"../insert/insert.h"
#include"../select/select.h"
#include"../create/create.h"
#include<time.h>

void start_API(t_log*);
e_query processQuery(char *, t_log*);

