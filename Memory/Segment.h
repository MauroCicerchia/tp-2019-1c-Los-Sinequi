#include"Page.h"
#include<stdio.h>
#include<stdlib.h>

typedef struct{
	char* segment_id;
	t_list* page_list;
}segment;

segment* segment_create();
