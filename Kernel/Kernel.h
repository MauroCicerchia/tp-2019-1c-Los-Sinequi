#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sharedLib/console.h>
#include<sharedLib/client.h>

e_query processQuery(char *, t_log*);

void iniciar_logger(t_log **logger)
{
	*logger = log_create("Kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
}

int iniciar_cliente() {
	return connectToServer();
//	sendMessages(server);
//	closeConnection(server);
}

void start_API(t_log *logger){
	char *input;
	input = readline(">");
	while(strcmp("", input)) {
		processQuery(input, logger);
		free(input);
		input = readline(">");
	}
}
