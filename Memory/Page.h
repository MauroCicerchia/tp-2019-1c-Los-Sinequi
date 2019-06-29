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
	int last_time_used;
}page;

//pageData* pageData_create();
//pageData* create_load_pageData(int, int, char*);
page* page_create();
page* create_load_page(int,int,int);
void page_destroy(void*);

