#include<sharedLib/console.h>
#include<sharedLib/client.h>
#include<sharedLib/query.h>
#include<sharedLib/packaging.h>
#include<sharedLib/consistency.h>
#include<stdlib.h>
#include<commons/config.h>
#include<commons/collections/list.h>

typedef struct{
	char* tableName;
	char* consType;
	char* partNum;
	char* compTime;
}metadata;

void metadata_destroy(void*);
char* send_select_to_FS(char*,uint16_t,t_log*);

int send_create_to_FS(char*,char*,char*,char*,t_log*);
t_list *send_describe_to_FS(char*,t_log*);

int send_insert_to_FS(char*,uint16_t,uint64_t,char*,t_log*);
int send_drop_to_FS(char*,t_log*);

int connect_to_FS(t_log*);
