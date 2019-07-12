#include"Page.h"


page* page_create(){
	page* segmentPage;
	segmentPage= (page*) malloc(sizeof(page));
	return (segmentPage);
}

page* create_load_page(int pageNum,int frame_num,int modified){
	page* segmentPage = page_create();
	segmentPage->page_num=pageNum;
	segmentPage->frame_num=frame_num;
	segmentPage->isModified=modified;
	segmentPage->last_time_used=get_timestamp();
	return segmentPage;
}

void page_destroy(void* aPage){
	free(aPage);
}

uint64_t get_timestamp(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	uint64_t  x = (uint64_t)( (tv.tv_sec)*1000 + (tv.tv_usec)/1000 );
	return x;
}

