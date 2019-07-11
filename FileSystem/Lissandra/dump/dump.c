#include"dump.h"

//dumpea todas las tablas de la memtable a los archivos correspondientes
void dump(){ //averiguar por semaforos
	char *toDump, *pTimestamp, *pKey, *pValue, *lfs;
	Itable *pTable; Iinsert *pInsert;
	int size = list_size(memtable);
	if(size != 0){
		log_info(logger,"[Dump]: Hay informacion para dumpear");
		log_info(logger,"[Dump]: Dumpeando...");
		for(int i=0; i<size; i++){ //recorro todas las tablas
			toDump = string_new();
			pTable = (Itable*)list_get(memtable,i);
			for(int j=0;j<list_size(pTable->inserts);j++){
				//recorro todos los inserts de la tabla actual
				pInsert = list_get(pTable->inserts,j);

				pTimestamp = string_duplicate(pInsert->timestamp);
				pKey = string_duplicate(pInsert->key);
				pValue = string_duplicate(pInsert->value);
				lfs = toLFSmode(pTimestamp,pKey,pValue);
				string_append(&toDump, lfs);
				free(pKey); free(pTimestamp); free(pValue); free(lfs);
			}
			fs_toDump(pTable->table,toDump);
			free(toDump);
		}
		mt_clean();
		log_info(logger,"[Dump]: Dump exitoso");
		tmpNo++;
	}
	else{
		log_warning(logger,"[Dump]: No hay nada que dumpear");
	}
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
