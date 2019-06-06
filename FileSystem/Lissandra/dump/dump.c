#include"dump.h"

//dumpea todas las tablas de la memtable a los archivos correspondientes
void dump(){ //averiguar por semaforos
	char *toDump, *pTimestamp, *pKey, *pValue;
	Itable *pTable;
	Iinsert *pInsert;
	t_list *pMemtable = list_duplicate(memtable);
//	mt_clean(memtable);
	for(int i=0; i<list_size(pMemtable); i++){ //recorro todas las tablas
		toDump = string_new();
		pTable = (Itable*)list_get(pMemtable,i);
		for(int j=0;j<list_size(pTable->inserts);j++){ //recorro todos los inserts de la tabla actual
			pKey = string_new(); pTimestamp = string_new(); pValue = string_new();
			pInsert = list_get(pTable->inserts,j);
			strcpy(pTimestamp,pInsert->timestamp);
			strcpy(pKey,pInsert->key);
			strcpy(pValue,pInsert->value);
			string_append(&toDump, toLFSmode(pTimestamp,pKey,pValue) );
			free(pKey); free(pTimestamp); free(pValue);
		}
		fs_toDump(pTable->table,toDump);
		free(toDump);
	}
	mt_cleanPivot(pMemtable);
}

char *toLFSmode(char *timestamp,char *key,char *value){
	char *lfsString = string_new();
	string_append(&lfsString,timestamp);
	string_append(&lfsString,";");
	string_append(&lfsString,key);
	string_append(&lfsString,";");
	string_append(&lfsString,value);
	string_append(&lfsString,"\n");
	return lfsString;
}
