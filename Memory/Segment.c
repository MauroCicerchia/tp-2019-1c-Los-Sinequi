#include"Segment.h"

segment* segment_create(){
	segment* memorySegment;
	memorySegment= (segment*) malloc(sizeof(segment));
	return(memorySegment);
}

bool segment_Pages_Available(segment* segmentFound){
	return list_size(segmentFound->page_list)<20; //suponemos que cada segmento puede tener 20 pag max

}

bool segment_Full(segment* segmentFound){
		bool isModified(void* aPage){
			return ((page*) aPage)->isModified;
		}
		return list_all_satisfy(segmentFound->page_list,isModified);
	}

void segment_add_page(segment* segment, int key, char* value){
	page* segmentPage = create_load_page(list_size(segment->page_list),
			create_load_pageData(mockitoTimestamp(),key,value),1);
	list_add(segment->page_list,segmentPage);
}


