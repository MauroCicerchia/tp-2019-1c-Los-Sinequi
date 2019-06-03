#include"memtable.h"

void mt_insert(char *table,char* timestamp, char *key,char *value){
	if(!mt_tableExists(table)){
		Itable *newTable;
		newTable->table = string_new();
		strcpy(newTable->table,table);
		newTable->inserts = list_create();
		list_add(newTable);
	}
	t_list *tableToInsert = mt_getTableToInsert(table);
	if(tableToInsert == NULL){printf("error con memtable\n"); break;}
	mt_addNewInsert(tableToInsert,timestamp,key,value);
}

t_list mt_getTablePosition(char *table){
	Itable pTable;
	for(int i=0;i<list_size(memtable);i++){
		pTable = (Itable*)list_get(memtable,i);
		if(pTable.table == table) return pTable.inserts;
	}
	return NULL;
}

void mt_addNewInsert(t_list tableToInsert, char *timestamp, char *key, char *value){
	insert *pInsert;
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
void tableDestroyer(Itable *table){
	free(table->table);
	list_destroy_and_destroy_elements(table->inserts,insertDestroyer);
}
void insertDestroyer(insert *insert){
	free(insert->key);
	free(insert->timestamp);
	free(insert->value);
}
