#include<stdlib.h>
#include<sharedLib/client.h>
#include<sharedLib/packaging.h>
#include<commons/txt.h>
#include"Memory.h"
#include"Table.h"

void qSelect(char*, uint16_t, t_log*);
void qInsert(char*, uint16_t, char*, t_log*);
void qCreate(char*, char*, char*, char*, t_log*);
void qDescribe(char*, t_log*);
void qDrop(char*, t_log*);
void qJournal(t_memory*, t_log*);
void output_describe(char*, e_cons_type, int, int);
void output_select(char*, uint16_t, char*);
