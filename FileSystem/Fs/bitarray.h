#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include"fcntl.h"
#include<semaphore.h>
#include"sys/stat.h"
#include"sys/mman.h"
#include"sys/types.h"


#include"commons/bitarray.h"
#include"commons/config.h"
#include"commons/txt.h"
#include"commons/log.h"
#include<pthread.h>
#include"fs.h"

extern char *absoluto;
extern t_log *logger;
extern int metadataBlocks;
extern int metadataSizeBlocks;
extern t_bitarray *bitarray;
extern int bitarrayfd;
extern char *bitarrayContent;
extern int lastBlockAssigned;
extern int flagBloquesLibres;

extern pthread_mutex_t MUTEX_BITARRAY;


#ifndef BITARRAY_H
#define BITARRAY_H

void ba_create();
int ba_exists();
int get_blocks_cuantity();
//void *ba_load_lfsMetadata();
int ba_getNewBlock();
void ba_freeBlock(int block);
void ba_loadBitarray();
void ba_bitarrayDestroy();
#endif
