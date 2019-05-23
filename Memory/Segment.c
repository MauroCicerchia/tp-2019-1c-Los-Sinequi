#include"Segment.h"

segment* segment_create(){
	segment* memorySegment;
	memorySegment= (segment*) malloc(sizeof(segment));
	return(memorySegment);
}
