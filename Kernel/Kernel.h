#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include"console.h"
#include"client.h"

e_query processQuery(char *, t_log*);

void iniciar_logger(t_log **logger)
{
	*logger = log_create("Kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
}
