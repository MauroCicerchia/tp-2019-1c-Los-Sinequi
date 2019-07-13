#include"Page.h"
#include<stdio.h>
#include<stdlib.h>


typedef struct{
	char* segment_id;
	t_list* page_list;
}segment;

segment* segment_create();
bool segment_Pages_Available(segment*);
bool segment_full(void*);
void segment_add_page(segment*,int,int);
void segment_destroy(void*);
void remove_page_from_segment(page*,segment*);
segment* segment_duplicate(segment*);
