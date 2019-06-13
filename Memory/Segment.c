#include"Segment.h"

segment* segment_create(){
	segment* memorySegment;
	memorySegment= (segment*) malloc(sizeof(segment));
	return(memorySegment);
}

bool segment_Pages_Available(segment* segmentFound){
	return list_size(segmentFound->page_list)<20; //suponemos que cada segmento puede tener 20 pag max

}

bool segment_full(void* segmentFound){
		bool isModified(void* aPage){
			return ((page*) aPage)->isModified;
		}
		return list_all_satisfy(((segment*)segmentFound)->page_list,isModified);
	}

void segment_add_page(segment* segment,int frame_num){
	page* segmentPage = create_load_page(list_size(segment->page_list),frame_num);
	list_add(segment->page_list,segmentPage);
}

void segment_destroy(void* aSegment){

//	free(((segment*) aSegment)->segment_id);
	list_destroy_and_destroy_elements(((segment*) aSegment)->page_list,page_destroy);
	free(aSegment);

}

