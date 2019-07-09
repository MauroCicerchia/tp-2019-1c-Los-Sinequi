#include<stdlib.h>
#include<sharedLib/client.h>
#include<sharedLib/packaging.h>
#include<commons/txt.h>
#include"Memory.h"
#include"Table.h"

int qSelect(char*, uint16_t, t_log*);
int qInsert(char*, uint16_t, char*, t_log*);
int qCreate(char*, char*, char*, char*, t_log*);
int qDescribe(char*, t_log*);
int qDrop(char*, t_log*);
int qJournal(t_memory*, t_log*);
void output_describe(char*, e_cons_type, int, int);
void output_select(char*, uint16_t, char*);
