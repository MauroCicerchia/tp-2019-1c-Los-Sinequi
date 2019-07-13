#include"compactador.h"

void *threadCompact(char *tableName)
{
	char log_msg[100];
	activeTable *table;
	while(tableIsActive(tableName)){

		table = com_getActiveTable(tableName);

		pthread_mutex_lock(&table->MUTEX_DROP_TABLE);
			table->ctime = com_getCTime(table->name); //actualiza cambios en el tiempo de compactacion
			compact(table);
		pthread_mutex_unlock(&table->MUTEX_DROP_TABLE);

			usleep(table->ctime * 1000);
		}

	sprintf(log_msg,"[Compactador %s]: Tabla quitada de lista de tablas activas", tableName);
	log_info(logger,log_msg);
	log_info(logger,"Terminando hilo de compactador...");
	free(tableName);
	return NULL;
}


void compact(activeTable *table)//agregar el semaforo para drop
{
	char log_msg[100];
	char *tableUrl = makeTableUrl(table->name);

	//conseguir todos los .tmp
	t_list *tmps = getAllTmps(tableUrl);

	if(list_size(tmps) == 0){
		sprintf(log_msg,"[Compactador %s]: No hay temporales para compactar", table->name);
		log_warning(logger,log_msg);
		list_destroy(tmps);
		free(tableUrl);
		return;
	}

	sprintf(log_msg,"[Compactador %s]: Hay temporales para compactar", table->name);
	log_warning(logger,log_msg);
	sprintf(log_msg,"[Compactador %s]: Compactando...", table->name);
	log_warning(logger,log_msg);

	//cambiar a .tmpc
	t_list *tmpsC = com_changeTmpsExtension(tmps,tableUrl);

	list_destroy_and_destroy_elements(tmps, free);

	//hacer el cambio del tmpc a la/las particiones correspondientes
	com_compactTmpsC(tmpsC,tableUrl,table);

	list_destroy_and_destroy_elements(tmpsC, free);

	//borrar los .tmpc y libera los bloques
	fs_cleanTmpsC(tableUrl);

	sprintf(log_msg,"[Compactador %s]: Compactacion terminada", table->name);
	log_warning(logger,log_msg);

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


	list_sort(allInserts,com_biggerTimeStamp); //ordeno la lista por timestamp de mayor a menor

	pthread_mutex_lock(&table->MUTEX_TABLE_PART);
	com_saveInPartition(keys,allInserts,table); //tomo la primera de cada key y la guardo en la particion
	pthread_mutex_unlock(&table->MUTEX_TABLE_PART);

	list_destroy_and_destroy_elements(keys,free);
	list_destroy_and_destroy_elements(allInserts,free);
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
	for(int i = 0; i < sizeofArray(args); i++){free(args[i]);}
	free(args);
	free(strTimeStamp);
	return timestamp;
}


char *com_key(char *insert)
{
	char **args = string_split((char*)insert, ";");
	char *strKey = string_duplicate(args[1]);

	for(int i = 0; i < sizeofArray(args); i++){
		free(args[i]);
	}
	free(args);

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

		pthread_mutex_lock(&MUTEX_ELSOLUCIONADOR);
			rename(tmpUrl,tmpcUrl); //renombras
		pthread_mutex_unlock(&MUTEX_ELSOLUCIONADOR);

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
		if(com_gotKey(key,(char*)insert))
			return true;
		else
			return false;

	}

	for(int i = 0; i < list_size(keys); i++){
		key = list_get(keys,i);
		toInsert = string_duplicate(list_find(allInserts,  _gotKey));

		partUrl = string_duplicate(tableUrl);
		iKey = strtol(key,NULL,10);
		part = string_itoa(iKey % tableParts);
		string_append(&partUrl,part);
		string_append(&partUrl,".bin");

		string_append(&toInsert,"\n");


		b_saveData(partUrl,toInsert);


		free(partUrl);
		free(toInsert);
		free(part);
	}
	free(tableUrl);
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
