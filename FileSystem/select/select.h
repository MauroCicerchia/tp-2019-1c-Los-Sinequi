#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<commons/txt.h>


typedef struct{
	int timeStamp;
	uint16_t key;
	char *value;
}dataSelect;

char *qselect(char *, uint16_t);

void loadList(t_list *,FILE *);

char *getValue(t_list *,uint16_t);

t_list *listToDATAmode(t_list *);

bool biggerTimeStamp(dataSelect, dataSelect);

bool isLastKey(uint16_t ,void*);

void *elemToDATAmode(void *);
