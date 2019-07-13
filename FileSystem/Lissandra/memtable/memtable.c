#include"memtable.h"

void mt_insert(char *table,char* timestamp, char *key,char *value)
{	//inserto un elemento en la memtable
	if(!mt_tableExists(table)){
		Itable *newTable = malloc(sizeof(Itable));
		newTable->table = string_duplicate(table);
		newTable->inserts = list_create();
		list_add(memtable,newTable);
	}
	t_list *tableToInsert = mt_getTableToInsert(table); //puntero a la lista de "inserts" para agregar uno nuevo
	mt_addNewInsert(tableToInsert,timestamp,key,value); //agrega el nuevo insert

}

//evalua si la tabla esta en la memtable
bool mt_tableExists(char *table)
{
	Itable *pTable;
	if(list_size(memtable) == 0)return false; //caso de que no exista

	for(int i=0;i<list_size(memtable);i++){
		pTable = (Itable*)list_get(memtable,i);
		if(!strcmp(pTable->table,table)){
			return true;
		}
	}
	return false;
}

//devuelve el puntero a la lista de "inserts" para agregar uno nuevo
t_list *mt_getTableToInsert(char *table)
{
	Itable *pTable;
	for(int i=0;i<list_size(memtable);i++){
		pTable = (Itable*)list_get(memtable,i);
		if(!strcmp(pTable->table,table)){
			return (t_list*)pTable->inserts;
		}
	}
	return list_create();
}

//agrega el nuevo insert a la lista de "inserts"
void mt_addNewInsert(t_list *tableToInsert, char *timestamp, char *key, char *value)
{
	Iinsert *pInsert = malloc(sizeof(Iinsert));
	pInsert->key = string_duplicate(key);
	pInsert->timestamp = string_duplicate(timestamp);
	pInsert->value = string_duplicate(value);

	list_add(tableToInsert,pInsert); //agregar insert
}

//vacia una tabla tipo memtable
void mt_clean()
{
	list_destroy_and_destroy_elements(memtable,tableDestroyer);
	memtable = list_create();
}

void mt_cleanPivot(t_list *tableToClean)
{
	list_destroy_and_destroy_elements(tableToClean,tableDestroyer);
}
void tableDestroyer(void* table)
{
	free(((Itable*)table)->table);
	list_destroy_and_destroy_elements(((Itable*)table)->inserts,insertDestroyer);
	free(table);
}
void insertDestroyer(void *insert)
{
	free(((Iinsert*)insert)->key);
	free(((Iinsert*)insert)->timestamp);
	free(((Iinsert*)insert)->value);
	free(insert);
}

void mt_getListofInserts(char *table, t_list *list)
{
	pthread_mutex_lock(&MUTEX_MEMTABLE);
	t_list *mtinserts = mt_getTableToInsert(table);

	if(list_size(mtinserts) == 0) {
		list_destroy(mtinserts);
		pthread_mutex_unlock(&MUTEX_MEMTABLE);
		return;
	}

	//bloqueo memtable mientras hace la asignacion
		for(int i = 0; i < list_size(mtinserts); i++){
			Iinsert *structInsert = list_get(mtinserts,i);
			char *pivot = string_new();
			string_append(&pivot,structInsert->timestamp);
			string_append(&pivot, ";");
			string_append(&pivot,structInsert->key);
			string_append(&pivot,";");
			string_append(&pivot,structInsert->value);
			string_append(&pivot,"\n");

			list_add(list,string_duplicate(pivot));
			free(pivot);
		}
	pthread_mutex_unlock(&MUTEX_MEMTABLE);
}

