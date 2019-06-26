#include<sharedLib/console.h>
#include<sharedLib/client.h>
#include<sharedLib/query.h>
#include<sharedLib/packaging.h>
#include<sharedLib/consistency.h>
#include<stdlib.h>
#include<commons/config.h>

char* send_select_to_FS(char*,int,t_config*,t_log*);
void send_create_to_FS(char*,char*,int,int,t_config*,t_log*);
void send_describe_to_FS(char*,t_config*,t_log*);
void send_insert_to_FS(char*,int,char*,t_config*,t_log*);
void send_drop_to_FS(char*,t_config*,t_log*);

int connect_to_FS(t_config*,t_log*);
