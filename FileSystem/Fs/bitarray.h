#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include"commons/bitarray.h"
#include"commons/config.h"
#include"commons/txt.h"
#include"fs.h"

extern char *absoluto;
extern t_config *lfsMetadata;

#ifndef BITARRAY_H
#define BITARRAY_H

void ba_create();
int ba_exists();
int get_blocks_cuantity();
void *ba_load_lfsMetadata();

#endif
