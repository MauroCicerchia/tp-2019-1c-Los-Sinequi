#include<stdio.h>
#include<stdlib.h>

typedef struct{
	int timestamp;
	int key;
	char value[50];
}pageData;

typedef struct{
	int page_num;
	pageData* page_data;
	bool isModified;
}page;

page page_create(){
	page segmentPage;
	segmentPage= (page*) malloc(sizeof(page));
	segmentPage.page_data= pageData_create();
	return (segmentPage);

}
pageData pageData_create(){
	pageData pageInfo;
	pageInfo= (pageData*) malloc(sizeof(pageData));
	return (pageInfo);
}
