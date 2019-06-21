#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<sharedLib/console.h>
#include<sharedLib/server.h>
#include<sharedLib/query.h>
#include<sharedLib/packaging.h>
#include<sharedLib/consistency.h>

extern char *ip;
extern char *port;
extern t_log *logger;

void *listen_client();
void process_query_from_client(int client);
