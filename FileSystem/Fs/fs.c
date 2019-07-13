#include"fs.h"


int fs_tableExists(char* table)
{
	char *tableUrl = makeUrlForPartition(table,"0");
	pthread_mutex_lock(&MUTEX_ELSOLUCIONADOR);
	if(access(tableUrl,F_OK) != -1){
		free(tableUrl);
		pthread_mutex_unlock(&MUTEX_ELSOLUCIONADOR);
		return 1;
	}
	else {
		free(tableUrl);
		pthread_mutex_unlock(&MUTEX_ELSOLUCIONADOR);
		return 0;
	}
}


int fs_create(char *table,char *consistency,int parts,int ctime)
{
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
	char *tableUrl = makeTableUrl(table);
	b_loadPartitionsFiles(tableUrl,parts); //le asigna el size y un bloque a cada bloque de particion
	free(tableUrl);
	log_info(logger, "  Les asigno un bloque inicial a cada particion");
	loadMetadata(table,consistency,parts,ctime);
	log_info(logger, "  Creo y cargo el archivo de metadata");
	return 1;
}

char *makeUrlForPartition(char *table,char *partition)
{
	char *url = makeTableUrl(table);
	string_append(&url,partition);
	string_append(&url,".bin");
	return url;
}

char *makeTableUrl(char *table)
{
	char *url = string_new();
	string_append(&url,absoluto);
	string_append(&url,"Tables/");
	string_append(&url,table);
	string_append(&url,"/");
	return url;
}

void makeDirectories(char *table)
{
	char *url = string_new();
	string_append(&url,absoluto);
	string_append(&url,"Tables/");
	string_append(&url,table);
	mkdir(url,0777);
	free(url);
}

void makeFiles(char *table,int parts)
{
	char *url;
	char* j;
	FILE *file;
	for(int i = 0;i<parts; i++){
		j = string_itoa(i);
		url = makeUrlForPartition(table,j);
		file = fopen(url,"w+");
		fclose(file);
		free(url);
		free(j);
	}
}

void makeMetadataFile(char *table)
{
	FILE *file;
	char *url = makeTableUrl(table);
	string_append(&url,"Metadata.bin");
	file = fopen(url,"w+");
	fclose(file);
	free(url);
}

void loadMetadata(char *table,char *consistency,int parts,int ctime)
{
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
	free(pctime); free(pparts);
}


void fs_toDump(char *table,char *toDump)
{
	activeTable* t = getActiveTable(table);
	pthread_mutex_lock(&t->MUTEX_TABLE_PART);
	char *tmpUrl = makeTableUrl(table);
	char *x = string_itoa(tmpNo);
	string_append(&tmpUrl,x);
	free(x);
	string_append(&tmpUrl,".tmp");

	FILE *file = txt_open_for_append(tmpUrl);
	fclose(file);

	b_assignSizeAndBlock(tmpUrl,0); //le asigno un bloque y un size 0

	b_saveData(tmpUrl,toDump); //guarda en la url tableUrl el char* que se le pasa

	free(tmpUrl);
	pthread_mutex_unlock(&t->MUTEX_TABLE_PART);
}


//devuelve un struct con la metadata de la tabla que se le pasa por param
metadata *fs_getTableMetadata(char *table)
{
	metadata *tableMetadata = malloc(sizeof(metadata));

	char *url = makeTableUrl(table);
	string_append(&url,"Metadata.bin");

	t_config *tableMetadataCfg;
	tableMetadataCfg = load_metadataConfig(url);
	if(tableMetadataCfg == NULL){
		log_error(logger,"[DESCRIBE]: Error con el archivo de Metadata de tabla");
		return NULL;
	}


	tableMetadata->consistency = getConsistency(tableMetadataCfg);
	tableMetadata->ctime = getCTime(tableMetadataCfg);
	tableMetadata->partitions = getPartitions(tableMetadataCfg);

	config_destroy(tableMetadataCfg);

	free(url);

	return tableMetadata;
}

t_config *load_metadataConfig(char *url)
{
	t_config *metadataCfg = config_create(url);
	if(metadataCfg == NULL){
		log_error(logger,"No se pudo abrir el archivo de metadata");
		return NULL;
	}
	return metadataCfg;

}

char *getConsistency(t_config *config)
{

	return string_duplicate(config_get_string_value(config,"CONS"));
}

char *getCTime(t_config *config)
{
	int ctime = config_get_int_value(config,"CTIME");
	return string_itoa(ctime);
}

char *getPartitions(t_config *config)
{
	int parts =config_get_int_value(config,"PARTS");
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

activeTable *getActiveTable(char *tableName){
	bool _tableIsActive(void *table){
		return !strcmp(tableName,((activeTable*)table)->name);
	}

	pthread_mutex_lock(&MUTEX_LISTACTIVETABLES);
	activeTable *x = (activeTable*)list_find(sysTables,_tableIsActive);
	pthread_mutex_unlock(&MUTEX_LISTACTIVETABLES);
	return x;
}

t_list *fs_getListOfInserts(char* table,int key)
{
	char *tableUrl = makeTableUrl(table);
	char *tableMetadataUrl, *partUrl, *tmpUrl;

	tableMetadataUrl = string_new();
	string_append(&tableMetadataUrl,tableUrl);
	string_append(&tableMetadataUrl,"Metadata.bin");
	t_config *tableMetadataCfg = load_metadataConfig(tableMetadataUrl);

	char *strpartitions = getPartitions(tableMetadataCfg);
	char *partition = string_itoa(key % strtol(strpartitions,NULL,10));
	free(strpartitions);

	config_destroy(tableMetadataCfg);

	partUrl = string_new();
	string_append(&partUrl,tableUrl);
	string_append(&partUrl,partition);
	string_append(&partUrl, ".bin");
	free(partition);

	t_list *partList = list_create();

	activeTable *x = getActiveTable(table);

	pthread_mutex_lock(&x->MUTEX_TABLE_PART);
		b_getListOfInserts(partUrl,partList); //trae todas los inserts de esa url (la de particion adecuada)

		t_list *tmps = getAllTmps(tableUrl);

		for(int i= 0; i < list_size(tmps); i++){
			tmpUrl = string_new();
			string_append(&tmpUrl,tableUrl);
			string_append(&tmpUrl,list_get(tmps,i));
			b_getListOfInserts(tmpUrl,partList);
			free(tmpUrl);
		}
	pthread_mutex_unlock(&x->MUTEX_TABLE_PART);


	mt_getListofInserts(table,partList); //toma todos los inserts de la memtable referidos a la tabla

	list_destroy_and_destroy_elements(tmps, free);
	free(partUrl);
	free(tableMetadataUrl);
	free(tableUrl);

	return partList;
}


//busca en la url de la tabla todos los .tmp y devuelve el "nombre.tmp"
t_list *getAllTmps(char *tableUrl)
{
	t_list *allTmpsNames = list_create();
	char *table;

	DIR *d;
	struct  dirent *dir;

	d = opendir(tableUrl);
	dir = readdir(d);
	while(dir != NULL){
		if(string_ends_with(dir->d_name,".tmp")){
			table = string_duplicate(dir->d_name);
			list_add(allTmpsNames,table);
		}

		dir = readdir(d);
	}

	closedir(d);
	return allTmpsNames;
}

void fs_setActualTmps()
{
	char *url;

	t_list *alltmps; //tmps de una tabla
	t_list *allTables = fs_getAllTables();
	for(int i = 0; i < list_size(allTables); i++){ //recorro todas las tablas
		char *table = list_get(allTables,i);
		url = makeTableUrl(table);
		alltmps = getAllTmps(url);
		if(list_size(alltmps) != 0){
			incrementTmpNo(alltmps);
		}

		list_destroy_and_destroy_elements(alltmps, free);
		free(url);

	}
	list_destroy_and_destroy_elements(allTables, free);
}

void incrementTmpNo(t_list *alltmps)
{
	char *tmp;
	char **aux;
	int n;
	for(int i = 0; i < list_size(alltmps); i++){
		tmp = string_duplicate(list_get(alltmps,i));
		aux = string_split(tmp, ".");
		n = strtol(aux[0],NULL,10);
		if(n > tmpNo )
			tmpNo = n;
		free(tmp);
		free(aux[0]);free(aux[1]);free(aux[2]);free(aux);
	}
}

t_list *fs_getAllTables()
{
	t_list *allTables = list_create();
	char *table;

	DIR *d;
	struct  dirent *dir;

	char *url = string_new();
	string_append(&url, absoluto);
	string_append(&url,"Tables/");

	d = opendir(url);
	dir = readdir(d);
	while(dir != NULL){
		if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")){
			table = string_duplicate(dir->d_name);
			list_add(allTables,table);
		}

		dir = readdir(d);
	}

	free(url);
	closedir(d);
	return allTables;

}

void fs_cleanTmpsC(char *tableUrl){
	char *file,*strBlocks;
	DIR *d;
	struct  dirent *dir;
	char **blocks;
	int blockToFree;


	d = opendir(tableUrl);
	dir = readdir(d);
	while(dir != NULL){ //borro todos los archivos del directorio
		if(isTmpc(dir->d_name)){
			file = string_duplicate(tableUrl);
			string_append(&file,dir->d_name);

			strBlocks = getListOfBlocks(file);
			blocks = string_get_string_as_array(strBlocks);
			free(strBlocks);
			for(int i = 0; i < sizeofArray(blocks); i++){ //libero bloque por bloque, del bitarray y su contenido
				blockToFree = strtol(blocks[i],NULL,10);
				b_freeblock(blockToFree);
			}

			unlink(file); //borro el archivo
			free(file);
		}
		dir = readdir(d);
	}
	closedir(d);

	for(int j = 0; j < sizeofArray(blocks);j++){free(blocks[j]);}
	free(blocks);
}

bool isTmpc(char *string){
	return string_ends_with(string, ".tmpc");
}

void fs_setActiveTables()
{
	char *configUrl;
	activeTable *pivot;
	t_config *cfg;
	t_list *tables = fs_getAllTables();

	for(int i = 0; i < list_size(tables);i++){
		pivot = malloc(sizeof(activeTable));
		configUrl = makeTableUrl(list_get(tables,i));
		string_append(&configUrl,"Metadata.bin");

		cfg = load_metadataConfig(configUrl);

		pivot->name = string_duplicate(list_get(tables,i));
		pthread_mutex_init(&(pivot->MUTEX_DROP_TABLE),NULL);
		pthread_mutex_init(&(pivot->MUTEX_TABLE_PART),NULL);
		char *strCtime = getCTime(cfg);
		char *strParts = getPartitions(cfg);
		pivot->ctime = strtol(strCtime,NULL,10);
		pivot->parts = strtol(strParts,NULL,10);
		free(strCtime); free(strParts);

		list_add(sysTables,pivot);

		config_destroy(cfg);
		free(configUrl);
	}

	list_destroy_and_destroy_elements(tables,free);
}
