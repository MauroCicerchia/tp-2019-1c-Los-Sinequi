#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include"commons/bitarray.h"
#include"commons/config.h"
#include"commons/txt.h"
#include"commons/log.h"
#include"fs.h"

extern char *absoluto;
extern t_log *logger;
extern t_config *lfsMetadata;
extern t_bitarray *bitarray;
extern int lastBlockAssigned;

#ifndef BITARRAY_H
#define BITARRAY_H

void ba_create();
int ba_exists();
int get_blocks_cuantity();
void *ba_load_lfsMetadata();
int ba_getNewBlock();
void ba_freeBlock(int block);
#endif
