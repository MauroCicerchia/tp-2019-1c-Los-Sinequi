#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include"sys/stat.h"
#include<commons/config.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include<sharedLib/console.h>
#include"bitarray.h"

extern int metadataBlocks;
extern int metadataSizeBlocks;
extern pthread_mutex_t MUTEX_ELSOLUCIONADOR;

#ifndef BLOCKS_H
#define BLOCKS_H
void b_loadPartitionsFiles(char *tableUrl, int parts);   //le asigna el size y un bloque a cada bloque de particion
void b_assignSizeAndBlock(char *partUrl, int size) ;  //le asigna un bloque y el size a esa url
void b_getListOfInserts(char *partUrl, t_list *list); //trae todas los inserts de esa url, que es la particion.bin o el .tmp
void insertsToList(char *inserts,t_list *list);
void startPartition(char *url, int blockNumber,int size);
char *getListOfBlocks(char *partUrl);
int getSizeOfBlocks();
void b_modifyBlocks(char *partUrl, char *listBlocks);

void b_saveData(char *url,char *data);

int freeSizeOfTheFirstNotFullBlock(char *url);
int b_get_lastBlock(char *url);
int b_get_firstFreeBlock(char *url);
bool b_full(int block);
bool b_empty(int block);
void b_addNewBlock(char *url);
int b_freeSize(int block);
int b_realFreeSize(int block);
int b_freeSizeOfLastBlock(char *url);
void b_saveIntoBlock(char *blockUrl,char *data);
void b_updateSize(char *tmpUrl);
void b_modifySize(char *url,int tam);
void b_writeBlockAssigned(int block);
void b_freeblock(int block);
bool b_blocksCreated();
void b_create();

#endif
