#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/collections/list.h>
#include<stdbool.h>

typedef struct{
	int timestamp;
	int key;
	char* value;
}pageData;

typedef struct{
	int page_num; //key;
	pageData* page_data;//void* frame;
	int isModified;
}page;

pageData* pageData_create();
page* page_create();
page* create_load_page(int, pageData*,int);
pageData* create_load_pageData(int, int, char*);
void page_destroy(void*);
