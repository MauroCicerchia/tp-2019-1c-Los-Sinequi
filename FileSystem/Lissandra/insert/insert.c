#include"insert.h"

void qinsert(char *table, char* key, char *value, char* timeStamp){
	if(fs_tableExists(table))
	mt_insert(table,timeStamp,key,value);
	else printf("La tabla sobre la que se quiere hacer el insert no existe\n");
}

//recibe por parametro timestamp,key y value, los pasa a string y duelve un string de tipo "timestamp;key,value"
//char* toLFSmode(char* timeStamp, char* key, char *value){
//	char *insert = string_new();
//	string_append(&insert,timeStamp);
//	string_append(&insert, ";");
//	string_append(&insert, key);
//	string_append(&insert, ";");
//	string_append(&insert, value);
//	string_append(&insert, "\n");
//	return insert;
//}
