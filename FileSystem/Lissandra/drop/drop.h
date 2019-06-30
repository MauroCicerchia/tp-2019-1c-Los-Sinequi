#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>

#include"../../Fs/fs.h"
#include"../../Fs/blocks.h"
#include"../../Lissandra/compactador/activeTable.h"
#include"../../Lissandra/compactador/compactador.h"

#include"../delayer/delayer.h"

#ifndef DROP_H
#define DROP_H

int qdrop(char *table);
void deleteDirectoriesAndFiles(char *table);
void freeBlocks(char *table);
void deleteTableFromMemory(char *table);
t_list *getFiles(char *tableUrl);

#endif
