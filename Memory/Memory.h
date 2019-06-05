#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sharedLib/console.h>
#include<sharedLib/server.h>
#include"Segment.h"
#include<time.h>

t_list* segmentList;
e_query processQuery(char *, t_log*);

void conectar_Kernel(t_log*);
void iniciar_logger(t_log**);
int conectar_FS(t_log*);
void start_API(t_log*);
segment* search_segment(char*);
page* search_page(segment*,int);
char* selectM(char*,int,t_log*);	   // (nombreTabla,key)
void insertM(char*,int,char*,t_log*); // (nombreTabla,key,value)
segment* segment_init(t_log*);
void createM(char*,/*consistencia,*/int,int);

int mockitoTimestamp(){
	return 4;
}

