#include<stdlib.h>
#include<sharedLib/client.h>
#include<sharedLib/packaging.h>
#include<commons/txt.h>
#include"Memory.h"
#include"Table.h"

void qSelect(char**, t_log*);
void qInsert(char**, t_log*);
void qCreate(char**, t_log*);
void qDescribe(char**, t_log*);
void qDrop(char**, t_log*);
void output_describe(char*, e_cons_type, int, int);
