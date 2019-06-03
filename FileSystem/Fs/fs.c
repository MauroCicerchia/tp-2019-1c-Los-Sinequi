#include"fs.h"

char *absolutePath(){
	return "/home/utnso/workspace/tp-2019-1c-Los-Sinequi/FileSystem/"; //usar con consola
//	return ""; //usar con eclipse
}

int fs_tableExists(char* table){
	char *tableUrl = makeUrlForPartition(table,"0");
	if(access(tableUrl,F_OK) != -1){
		free(tableUrl);
		return 1;
	}
	else return 0;
}

int fs_create(char *table,char *consistency,int parts,int ctime){
	if(parts == 0){
		printf("No puede haber 0 particiones\n");
		return 0;
	}
	if(ctime == 0){
		printf("El tiempo de compactacion no puede ser 0\n");
		return 0;
	}
	makeDirectories(table);
	makeFiles(table,parts);
	makeMetadataFile(table);
	loadMetadata(table,consistency,parts,ctime);
	return 1;
}

char *makeUrlForPartition(char *table,char *partition){
	char *url = makeTableUrl(table);
	string_append(&url,partition);
	string_append(&url,".bin");
	return url;
}

char *makeTableUrl(char *table){
	char *url = string_new();
	char *abs = string_new();
	strcpy(url,absolutePath());
	string_append(&url,"mnt/tables/");
	string_append(&url,table);
	string_append(&url,"/");
	return url;
}

void makeDirectories(char *table){
	char *url = string_new();
	strcpy(url,absolutePath());
	string_append(&url,"mnt/tables/");
	string_append(&url,table);
	mkdir(url,0777);
}

void makeFiles(char *table,int parts){
	char *url;
	char* j;
	FILE *file;
	for(int i = 0;i<parts; i++){
		url = string_new();
		j = string_itoa(i);
		url = makeUrlForPartition(table,j);
		printf("%s\n",url);
		file = fopen(url,"w+");
		fclose(file);
		free(url);

	}
}

void makeMetadataFile(char *table){
	FILE *file;
	char *url = makeTableUrl(table);
	string_append(&url,"Metadata.bin");
	file = fopen(url,"w+");
	if (file != NULL)
	printf("archivo abierto\n");
	fclose(file);
	free(url);
}

void loadMetadata(char *table,char *consistency,int parts,int ctime){

	FILE *file;
	char *pctime = string_itoa(ctime);
	char *pparts = string_itoa(parts);
	char *url = makeTableUrl(table);
	string_append(&url,"Metadata.bin");

	file = txt_open_for_append(url);
		char *aux = string_new();
		string_append(&aux,"CONS=");
		string_append(&aux,consistency);
		string_append(&aux,"\n");
		txt_write_in_file(file,aux);
		free(aux);

		aux = string_new();
		string_append(&aux,"PARTS=");
		string_append(&aux,pparts);
		string_append(&aux,"\n");
		txt_write_in_file(file,aux);
		free(aux);

		aux = string_new();
		string_append(&aux,"CTIME=");
		string_append(&aux,pctime);
		string_append(&aux,"\n");
		txt_write_in_file(file,aux);
		free(aux);

	txt_close_file(file);
	printf("metadata cargada con exito");
}

