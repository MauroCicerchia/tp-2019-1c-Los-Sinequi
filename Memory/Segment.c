#include"Page.c"
#include<stdio.h>
#include<stdlib.h>

typedef struct{
	int segment_id;
	page* page_pointer;
}segment;

segment segment_create(){
	segment memorySegment;
	memorySegment= (segment*) malloc(sizeof(segment));
	return(memorySegment);
}
