#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<commons/string.h>
#include<commons/txt.h>
#include"../memtable/memtable.h"

#include "../../Fs/fs.h"

void qinsert(char*, char*, char*, char*);
char* toLFSmode(char*,char*,char*);
