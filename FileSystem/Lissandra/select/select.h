#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<commons/log.h>
//#include<commons/collections/list.h>
#include<commons/string.h>
#include<commons/txt.h>
#include"../../Fs/fs.h"

#include"../delayer/delayer.h"

extern t_log *logger;

#ifndef SELECT_H
#define SELECT_H

typedef struct{
	uint64_t timeStamp;
	uint16_t key;
	char *value;
}dataSelect;

char *qselect(char *, char*);

//void loadList(t_list *,FILE *);

char *getValue(t_list *,uint16_t);

t_list *listToDATAmode(t_list *);

bool biggerTimeStamp(void*, void*);

bool isLastKey(uint16_t ,void*);

void *elemToDATAmode(void *);

void dataSelect_destroy(void*);

#endif
