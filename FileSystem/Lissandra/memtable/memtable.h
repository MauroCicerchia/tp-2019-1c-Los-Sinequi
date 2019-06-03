#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<commons/txt.h>


typedef struct{
	char *timestamp;
	char *key;
	char *value;
}insert;

typedef struct{
	char *table;
	t_list *inserts;
}Itable;

void mt_clean();
void mt_tableExists();
void mt_addNewTable();
void mt_addNewInsert();

