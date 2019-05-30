#include"Page.h"
#include<stdio.h>
#include<stdlib.h>

typedef struct{
	char* segment_id;
	t_list* page_list;
}segment;

segment* segment_create();
bool segment_Pages_Available(segment*);
bool segment_Full(segment*);
void segment_add_page(segment*,int,char*);
