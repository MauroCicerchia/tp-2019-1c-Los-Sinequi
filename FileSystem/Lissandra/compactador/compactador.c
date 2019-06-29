#include"compactador.h"

void *threadCompact(char *tableName)
{
	activeTable *table;
	while(tableIsActive(tableName)){

		table = com_getActiveTable(tableName);
		sem_wait(&table->MUTEX_DROP_TABLE);
			table->ctime = com_getCTime(table->name); //actualiza cambios en el tiempo de compactacion
			compact(table);
		sem_post(&table->MUTEX_DROP_TABLE);

		sleep(table->ctime/1000);
	}


	free(tableName);
	return NULL;
}


void compact(activeTable *table)//agregar el semaforo para drop
{
	char *tableUrl = makeTableUrl(table->name);

	//conseguir todos los .tmp
	t_list *tmps = getAllTmps(tableUrl);


	if(list_size(tmps) == 0){
		log_warning(logger,"No hay temporales para compactar");
		return;
	}

	//cambiar a .tmpc
	t_list *tmpsC = com_changeTmpsExtension(tmps,tableUrl);

	list_destroy_and_destroy_elements(tmps, free);

	//hacer el cambio del tmpc a la/las particiones correspondientes
	com_compactTmpsC(tmpsC,tableUrl,table);

	//borrar los .tmpc y libera los bloques
	fs_cleanTmpsC(tableUrl);

	free(tableUrl);
}

void com_compactTmpsC(t_list *tmpsC,char *tableUrl, activeTable *table)
{
	t_list *allInserts = list_create();
	t_list *tmpInserts;
	char *tmpUrl;

	for(int i = 0; i < list_size(tmpsC); i++){
		tmpUrl = string_duplicate(tableUrl);
		string_append(&tmpUrl,list_get(tmpsC,i));
		tmpInserts = list_create();

		b_getListOfInserts(tmpUrl, tmpInserts);


		for(int j = 0; j < list_size(tmpInserts); j++){ //le paso todos los inserts de ese tmp a la lista ppal
			list_add(allInserts,string_duplicate(list_get(tmpInserts,j)));
		}

		list_destroy_and_destroy_elements(tmpInserts,free);
		free(tmpUrl);
	}

	t_list *keys = com_getAllKeys(allInserts); //guardo todas las keys posibles

	for(int k = 0; k < list_size(allInserts); k++){ // le agrego \n a todos los inserts
		char *insert =  list_get(allInserts,k);
		if(!string_ends_with(insert,"\n")){
			string_append(&insert,"\n");
		}
	}

	list_sort(allInserts,com_biggerTimeStamp); //ordeno la lista por timestamp de mayor a menor

	sem_wait(&table->MUTEX_TABLE_PART);
	com_saveInPartition(keys,allInserts,table); //tomo la primera de cada key y la guardo en la particion
	sem_post(&table->MUTEX_TABLE_PART);
}


bool com_biggerTimeStamp(void *elem1, void *elem2)
{
	return com_timestamp(elem1) > com_timestamp(elem2);
}


uint64_t com_timestamp(void *insert)
{
	char **args = string_split((char*)insert, ";");
	char *strTimeStamp = string_duplicate(args[0]);
	uint64_t timestamp = strtouq(strTimeStamp,NULL,10);
	free(args);
	free(strTimeStamp);
	return timestamp;
}


char *com_key(char *insert)
{
	char **args = string_split((char*)insert, ";");
	char *strKey = string_duplicate(args[1]);
//	uint16_t key = strtol(strKey,NULL,10);
	free(args);
//	free(strKey);
//	return key;
	return strKey;
}


//te devuelve la lista de todos los tmp ahora con la extension .tmpc
t_list *com_changeTmpsExtension(t_list *tmps, char *tableUrl)
{
	t_list *tmpsc = list_create();
	char *tmpcUrl,*tmpUrl, *tmp, *tmpc;

	for(int i = 0; i < list_size(tmps); i++){
		tmpUrl = string_duplicate(tableUrl);
		tmpcUrl = string_duplicate(tableUrl);

		tmp = string_duplicate(list_get(tmps,i));
		string_append(&tmpUrl,tmp);

		tmpc = string_duplicate(tmp);
		string_append(&tmpc,"c");
		string_append(&tmpcUrl,tmpc);

		rename(tmpUrl,tmpcUrl); //renombras

		list_add(tmpsc,tmpc); //agregas a la nueva lista

		free(tmp);
		free(tmpUrl); free(tmpcUrl);

	}

	return tmpsc;
}


//duelve una lista con todos las keys uint16_t que fueron insertadas
t_list *com_getAllKeys(t_list *inserts)
{
	t_list *keys = list_create();
	char *strKey;
//	uint16_t key;

	for(int i = 0; i < list_size(inserts); i++){
		strKey = com_key(list_get(inserts,i));
//		key = strtol(strKey,NULL,10);
		if(!keyIsAdded(strKey,keys))
			list_add(keys,string_duplicate(strKey));
		free(strKey);
	}

	return keys;
}

bool keyIsAdded(char *key,t_list *keys)
{
	char *listKey;
	for (int i = 0; i < list_size(keys); i++){
		listKey = string_duplicate(list_get(keys,i));
		if(!strcmp(key,listKey)){
			free(listKey);
			return true;
		}

	free(listKey);
	}
	return false;
}

/*
 * allInserts = lista ,ordenada por tsmp, de "tsmp;key;value"
 * keys = lista de uint16_t con las todas las key a guardar
 */
void com_saveInPartition(t_list *keys,t_list *allInserts, activeTable *table)
{
	char *key;
	char *tableUrl = makeTableUrl(table->name);
	char *partUrl,*part,*toInsert;
	int iKey;
	int tableParts = table->parts;

	bool _gotKey(void *insert){ //wrapper
		return com_gotKey(key,(char*)insert);
	}

	for(int i = 0; i < list_size(keys); i++){
		key = list_get(keys,i);
		toInsert = list_find(allInserts,  _gotKey);

		partUrl = string_duplicate(tableUrl);
		iKey = strtol(key,NULL,10);
		part = string_itoa(iKey % tableParts);
		string_append(&partUrl,part);
		string_append(&partUrl,".bin");

		b_saveData(partUrl,toInsert);

		free(partUrl);
	}
}

bool com_gotKey(char *key, char *insert)
{
	char *listKey = com_key(insert);
	bool boolean = !strcmp(listKey,key);
	free(listKey);
	return boolean;
}

activeTable *com_getActiveTable(char *tableName){
	bool _tableIsActive(void *table){
		return !strcmp(tableName,((activeTable*)table)->name);
	}

	return (activeTable*)list_find(sysTables,_tableIsActive);
}


bool tableIsActive(char *tableName)
{
	bool _equalName(void *table){
		return !strcmp(tableName,((activeTable*)table)->name);
	}

	return list_any_satisfy(sysTables,_equalName);
}


int com_getCTime(char *tableName)
{
	char *url = makeTableUrl(tableName);
	string_append(&url,"Metadata.bin");

	t_config *cfg = config_create(url);
	int cTime = config_get_int_value(cfg,"CTIME");
	config_destroy(cfg);

	free(url);
	return cTime;
}
