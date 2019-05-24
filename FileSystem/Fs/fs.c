#include"fs.h"

int fs_tableExists(char* table){
	char *tableUrl = makeUrl(table,"0");
	if(access(tableUrl,F_OK) != -1){
		free(tableUrl);
		return 1;
	}
	else return 0;
}

int fs_create(char *table,char *consistency,int parts,int ctime){
	if(parts == 0) return 0;
	if(ctime == 0) return 0;
	makeDirectoriesAndFiles(table,parts);
	makeMetadataFile(table);
	loadMetadata(table,consistency,parts,ctime);
	return 1;
}

char *makeUrl(char *tabla,char *partition){
	char *url =string_new();
	string_append(&url,"tables/");
	string_append(&url,tabla);
	string_append(&url,"/");
	string_append(&url,partition);
	string_append(&url,".bin");
	return url;
}

void makeDirectoriesAndFiles(char *table,int parts){
	char *url;
	int j = 0;
	FILE *file; //free?
	for(int i;i = 0; i<parts){
		url = malloc(sizeof(char)*50);
		string_append(&url,"tables/");
		string_append(&url,table);
		string_append(&url,"/");
		j = string_itoa(i);
		string_append(&url,j);
		string_append(&url,".bin");
		fopen(url,file);
		fclose(file);
		free(url);
	}
}

void makeMetadataFile(char *table){
	char *url = malloc(sizeof(char)*50);
	FILE *file; //free?
	string_append(&url,"tables/");
	string_append(&url,"Metadata.bin");
	file = fopen(url,"w");
	fclose(file);
	free(url);
}

void loadMetadata(char *table,char *consistency,int parts,int ctime){
	char *url = malloc(sizeof(char)*50);
	FILE *file; //free?
	char *pctime = string_itoa(ctime);
	char *pparts = string_itoa(parts);
	string_append(&url,"tables/");
	string_append(&url,"Metadata.bin");

	file = txt_open_for_append(url);
		char *aux = malloc(sizeof(char)*50);
		string_append(aux,"CONS = ");
		string_append(aux,consistency);
		txt_write_in_file(file,aux);
		free(aux);

		aux = malloc(sizeof(char)*50);
		string_append(aux,"PARTS = ");
		string_append(aux,pparts);
		free(aux);

		aux = malloc(sizeof(char)*50);
		string_append(aux,"CTIME = ");
		string_append(aux,pctime);
		free(aux);
	txt_close_file(file);
}

