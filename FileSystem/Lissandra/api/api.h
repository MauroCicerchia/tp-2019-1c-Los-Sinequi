#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<semaphore.h>
#include<commons/log.h>
#include<commons/config.h>
#include"commons/string.h"
#include<readline/readline.h>
#include<readline/history.h>
//#include"sharedLib/query.h"
//#include<sharedLib/consistency.h>
#include"../insert/insert.h"
#include"../select/select.h"
#include"../create/create.h"
#include"../describe/describe.h"
#include"../drop/drop.h"

#include"../delayer/delayer.h"

#include<sys/time.h>

extern t_log *logger;
extern t_config *config;
extern t_config *metadataCfg;
extern pthread_mutex_t MUTEX_RETARDTIME;

void *start_Api();
void processQuery(char *);
uint64_t getCurrentTime();
void delayer();
