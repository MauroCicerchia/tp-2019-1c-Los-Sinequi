#include"insert.h"

void qinsert(char *table, char* key, char *value, char* timeStamp){
	char *url =  string_new();
	string_append(&url,"tables/");
	string_append(&url,table);
	string_append(&url, ".bin");
	FILE *file = txt_open_for_append(url);
	char *toInsert;
	toInsert = toLFSmode(timeStamp,key,value);
	txt_write_in_file(file, toInsert);
	free(toInsert);
	txt_close_file(file);
	free(url);
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
