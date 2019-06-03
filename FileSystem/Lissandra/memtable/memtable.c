#include"memtable.h"

void mt_insert(char *table,char* timestamp, char *key,char *value){
	if(!mt_tableExists(table)){
		Itable *newTable = malloc(sizeof(Itable));
		newTable->table = string_new();
		strcpy(newTable->table,table);
		newTable->inserts = list_create();
		list_add(memtable,newTable);
	}
	t_list *tableToInsert = mt_getTableToInsert(table);
	mt_addNewInsert(tableToInsert,timestamp,key,value);
}

bool mt_tableExists(char *table){
	Itable *pTable = malloc(sizeof(Itable));
	for(int i=0;i<list_size(memtable);i++){
		pTable = (Itable*)list_get(memtable,i);
		if(!strcmp(pTable->table,table)) return true;
	}
	free(pTable);
	return false;
}

t_list *mt_getTableToInsert(char *table){
	Itable *pTable;
	for(int i=0;i<list_size(memtable);i++){
		pTable = (Itable*)list_get(memtable,i);
		if(!strcmp(pTable->table,table)){
			return (t_list*)pTable->inserts;
		}
	}
	return list_create();
}

void mt_addNewInsert(t_list *tableToInsert, char *timestamp, char *key, char *value){
	Iinsert *pInsert = malloc(sizeof(Iinsert));
	pInsert->key = string_new();
	pInsert->timestamp = string_new();
	pInsert->value = string_new();
	strcpy(pInsert->key,key);
	strcpy(pInsert->timestamp,timestamp);
	strcpy(pInsert->value,value);
	list_add(tableToInsert,pInsert);
}

void mt_clean(){
	list_destroy_and_destroy_elements(memtable,tableDestroyer);
	memtable = list_create();
}
void tableDestroyer(void* table){
	free(((Itable*)table)->table);
	list_destroy_and_destroy_elements(((Itable*)table)->inserts,insertDestroyer);
	free(table);
}
void insertDestroyer(void *insert){
	free(((Iinsert*)insert)->key);
	free(((Iinsert*)insert)->timestamp);
	free(((Iinsert*)insert)->value);
	free(insert);
}
