#include"fs.h"


int fs_tableExists(char* table){
	char *tableUrl = makeUrlForPartition(table,"0");
	if(access(tableUrl,F_OK) != -1){
		free(tableUrl);
		return 1;
	}
	else {
		free(tableUrl);
		return 0;
	}
}

int fs_create(char *table,char *consistency,int parts,int ctime){
	log_info(logger, "  Chequeo coherencia de particiones");
	if(parts == 0){
		log_error(logger,"No puede haber 0 particiones");
		return 0;
	}
	log_info(logger, "  Chequeo coherencia en tiempo de compactacion");
	if(ctime == 0){
		log_error(logger,"El tiempo de compactacion no puede ser 0");
		return 0;
	}
	makeDirectories(table);
	log_info(logger, "  Creo los directorios");
	makeFiles(table,parts);
	log_info(logger, "  Creo los archivos de particion");
	makeMetadataFile(table);
	b_loadPartitionsFiles(parts); //le asigna el size y un bloque a cada bloque de particion
	log_info(logger, "  Les asigno un bloque inicial a cada particion");
	loadMetadata(table,consistency,parts,ctime);
	log_info(logger, "  Creo y cargo el archivo de metadata");
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
	string_append(&url,absoluto);
	string_append(&url,"Tables/");
	string_append(&url,table);
	string_append(&url,"/");
	return url;
}

void makeDirectories(char *table){
	char *url = string_new();
	string_append(&url,absoluto);
	string_append(&url,"Tables/");
	string_append(&url,table);
	mkdir(url,0777);
	free(url);
}

void makeFiles(char *table,int parts){
	char *url;
	char* j;
	FILE *file;
	for(int i = 0;i<parts; i++){
		url = string_new();
		j = string_itoa(i);
		url = makeUrlForPartition(table,j);
		file = fopen(url,"w+");
		fclose(file);
		free(url);
		free(j);
	}
}

void makeMetadataFile(char *table){
	FILE *file;
	char *url = makeTableUrl(table);
	string_append(&url,"Metadata.bin");
	file = fopen(url,"w+");
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
	free(url);
}

void fs_toDump(char *table,char *toDump){
	char *tableUrl = makeTableUrl(table);
	string_append(&tableUrl,table);
	string_append(&tableUrl,tmpNo);
	string_append(&tableUrl,".tmp");

	FILE *file = txt_open_for_append(tableUrl);
	fclose(file);

	b_assignSizeAndBlock(tableUrl); //le asigno un bloque y un size

	b_saveData(tableUrl,toDump); //guarda en la url tableUrl el char* que se le pasa

	free(tableUrl);
}


//devuelve un struct con la metadata de la tabla que se le pasa por param
metadata *fs_getTableMetadata(char *table)
{
	metadata *tableMetadata = malloc(sizeof(metadata));

	char *url = makeTableUrl(table);
	string_append(&url,"Metadata.bin");


	if(load_metadataConfig(url) == NULL) return NULL;

	log_info(logger,"  Abro el archivo de metadata");

	tableMetadata->consistency = getConsistency();
	tableMetadata->ctime = getCTime();
	tableMetadata->partitions = getPartitions();

	log_info(logger,"  Guardo la metadata");

//	config_destroy(metadataCfg);

	free(url);

	return tableMetadata;
}

void *load_metadataConfig(char *url)
{
	metadataCfg = config_create(url);
	if(metadataCfg == NULL){
		log_error(logger,"No se pudo abrir el archivo de metadata");
		return NULL;
	}
	return "no soy null :)";

}

char *getConsistency()
{
	log_info(logger,"   Leo tipo de consistencia");

	return config_get_string_value(metadataCfg,"CONS");
}

char *getCTime()
{
	log_info(logger,"   Leo el tiempo de compactacion");

	int ctime =config_get_int_value(metadataCfg,"CTIME");
	return string_itoa(ctime);
}

char *getPartitions()
{
	log_info(logger,"   Leo la cantidad de particiones");

	int parts =config_get_int_value(metadataCfg,"PARTS");
	return string_itoa(parts);
}

char *fs_getBitmapUrl()
{
	char *url =string_new();
	string_append(&url,absoluto);
	string_append(&url,"Metadata/Bitmap.bin");
	return url;
}

char *fs_getlfsMetadataUrl()
{
	char *url = string_new();
	string_append(&url,absoluto);
	string_append(&url,"Metadata/Metadata.bin");
	return url;
}

char* fs_getBlocksUrl()
{
	char *url = string_new();
	string_append(&url,absoluto);
	string_append(&url,"Blocks/");
	return url;
}

void fs_createBlocks(int blocks)
{
	char *url = fs_getBlocksUrl();
	char *block;
	for(int i=0;i<blocks;i++){
		block = string_new();
		string_append(&block,url);
		string_append(&block,string_itoa(i));
		string_append(&block,".bin");
		FILE *f = fopen(block,"w+");
		fclose(f);
		free(block);
	}
	free(url);
}
