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
#include<sharedLib/query.h>
#include<sharedLib/packaging.h>
#include"Segment.h"
#include<time.h>

t_list* segmentList;
e_query processQuery(char *, t_log*);
t_log *logger;

void iniciar_logger();
void *listen_client();
void start_API();
segment* search_segment(char*);
page* search_page(segment*,int);
char* selectM(char*,int);	   // (nombreTabla,key)
void insertM(char*,int,char*,t_log*); // (nombreTabla,key,value)
segment* segment_init(t_log*);
void createM(char*,/*consistencia,*/int,int);

