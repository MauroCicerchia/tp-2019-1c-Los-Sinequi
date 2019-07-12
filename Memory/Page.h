#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/collections/list.h>
#include<stdbool.h>
#include<stdint.h>

/*typedef struct{
	int timestamp;
	uint16_t key;
	char* value;
}pageData;
*/

typedef struct{
	int page_num;
	int frame_num;
	int isModified;
	uint64_t last_time_used;
}page;

page* page_create();
page* create_load_page(int,int,int);
void page_destroy(void*);

uint64_t get_timestamp();

