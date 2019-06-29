#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<commons/string.h>
#include<commons/txt.h>

#include"../../Fs/fs.h"
#include"../memtable/memtable.h"

extern t_list *memtable;
extern char* absoluto;
extern int tmpNo;

void dump();
char *toLFSmode(char*,char*,char*);
