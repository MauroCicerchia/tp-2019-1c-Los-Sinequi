#include"Page.h"

pageData* pageData_create(){
	pageData* pageInfo;
	pageInfo= (pageData*) malloc(sizeof(pageData));

	return (pageInfo);
}

page* page_create(){
	page* segmentPage;
	segmentPage= (page*) malloc(sizeof(page));
	segmentPage->page_data = pageData_create();
	return (segmentPage);
}

page* create_load_page(int pageNum, pageData* pageData,int isModified){
	page* segmentPage = page_create();
	segmentPage->page_num=pageNum;
	segmentPage->page_data=pageData;
	segmentPage->isModified=isModified;
	return segmentPage;
}

pageData* create_load_pageData(int timestamp, int key, char* value){
	pageData* segmentPageData = pageData_create();
	segmentPageData->key=key;
	segmentPageData->timestamp=timestamp;
	segmentPageData->value=value;
	return segmentPageData;
}


