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

void segment_add_page(segment* segment,int frame_num, int modified){
	page* segmentPage = create_load_page(list_size(segment->page_list),frame_num,modified);
	list_add(segment->page_list,segmentPage);
}

void remove_page_from_segment(page* aPage, segment* segment){
	bool isPage(void* anotherPage){
			return (((page*)anotherPage)->page_num == aPage->page_num);
		}
		list_remove_and_destroy_by_condition(segment->page_list,isPage,page_destroy);
}

void segment_destroy(void* aSegment){
	free(((segment*) aSegment)->segment_id);
	list_destroy_and_destroy_elements(((segment*) aSegment)->page_list,page_destroy);
	free(aSegment);

}

segment* segment_duplicate(segment* aSegment){
	segment* segmentDup = (segment*) malloc(sizeof(segment));
	segmentDup->segment_id = string_duplicate(aSegment->segment_id);
	segmentDup->page_list = list_create();

}

