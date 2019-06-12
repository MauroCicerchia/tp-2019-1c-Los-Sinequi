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
#include<sharedLib/consistency.h>
#include<commons/bitarray.h>
#include"Segment.h"
#include<time.h>

void* main_memory;
t_bitarray* bitmap;
int get_value_size();
void THEGREATMALLOC();

t_list* segmentList;
e_query processQuery(char *, t_log*);
t_log *logger;
t_config *config;

void load_config();
void iniciar_logger();
segment* segment_init(t_log*);
void *listen_client();
void process_query_from_client(int);
void start_API();
segment* search_segment(char*);
page* search_page(segment*,int);

char* selectM(char*,int);	   // (nombreTabla,key)
int insertM(char*,int,char*); // (nombreTabla,key,value)
int createM(char*,e_cons_type,int,int);
int dropM(char*);

/*table_t *describeM(char*);
t_list *describeM();*/


/*	PUERTO=8000
    IP_FS="192.168.1.2"
    PUERTO_FS=8001
    IP_SEEDS=["192.168.1.3","192.168.1.4"]
    PUERTO_SEEDS=[8000,8001]
    RETARDO_MEM=600
    RETARDO_FS=600
    TAM_MEM=2048
    RETARDO_JOURNAL=60000
    RETARDO_GOSSIPING=30000
    MEMORY_NUMBER=1
    IP=127.0.0.1
 */
