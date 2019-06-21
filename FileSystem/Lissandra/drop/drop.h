#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include"../../Fs/fs.h"
#include"../../Fs/blocks.h"

#ifndef DROP_H
#define DROP_H

int qdrop(char *table);
void deleteDirectoriesAndFiles(char *table);
void freeBlocks(char *table);
void deleteTableFromMemory(char *table);
t_list *getFiles(char *tableUrl);

#endif
