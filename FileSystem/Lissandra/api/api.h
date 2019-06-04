#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<commons/log.h>
#include"commons/string.h"
#include<readline/readline.h>
//#include<sharedLib/consistency.h>
#include"../insert/insert.h"
#include"../select/select.h"
#include"../create/create.h"
#include<sys/time.h>


char **parseQuery(char*);
void start_API(t_log*);
e_query processQuery(char *, t_log*);
uint64_t getCurrentTime();
