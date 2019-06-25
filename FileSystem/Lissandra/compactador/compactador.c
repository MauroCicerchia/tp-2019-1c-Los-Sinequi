#include"compactador.h"

typedef struct{
	char *name;
	int parts;
	int ctime;
	sem_t MUTEX_TABLE_PART;
	sem_t MUTEX_DROP_TABLE;
}activeTable;

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

void com_compactTmpsC(t_list *tmpsC,char *tableUrl, activeTable *table, int parts)
{
	t_list *allInserts = list_create();
	t_list *tmpInserts;

	for(int i = 0; i < list_size(tmpsC); i++){
		tmpInserts = list_create();

		b_getListOfInserts(tableUrl, tmpInserts);

		for(int j = 0; j < list_size(tmpInserts); j++){ //le paso todos los inserts de ese tmp a la lista ppal
			list_add(allInserts,string_duplicate(list_get(tmpInserts,j)));
		}

		list_destroy_and_destroy_elements(tmpInserts,free);
	}

	t_list *keys = com_getAllKeys(allInserts); //guardo todas las keys posibles

	list_sort(allInserts,biggerTimeStamp); //ordeno la lista por timestamp de mayor a menor

	sem_wait(table->MUTEX_TABLE_PART);
	com_saveInPartition(keys,allInserts,table); //tomo la primera de cada key y la guardo en la particion
	sem_post(table->MUTEX_TABLE_PART);
}


bool biggerTimeStamp(void *elem1, void *elem2)
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


uint16_t com_key(char *insert)
{
	char **args = string_split((char*)insert, ";");
	char *strKey = string_duplicate(args[1]);
	uint16_t key = strtol(strKey,NULL,10);
	free(args);
	free(strKey);
	return key;
}


//te devuelve la lista de todos los tmp ahora con la extension .tmpc
t_list *com_changeTmpsExtension(t_list *tmps, char *tableUrl)
{
	t_list *tmpsc = list_create();
	char *tmpcUrl,*tmpUrl, *tmp, *tmpc;

	for(int i = 0; i < list_size(tmps); i++){
		tmpUrl = string_duplicate(tableUrl);
		tmpcUrl = string_duplicate(tableUrl);

		tmp = list_get(tmps,i);
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
	uint16_t key;

	for(int i = 0; i < list_size(inserts); i++){
		key = com_key(list_get(inserts,i));
		if(!keyIsAdded(key,keys))
			list_add(keys,key);
	}

	return keys;
}

bool keyIsAdded(uint16_t key,t_list *keys)
{
	for (int i = 0; i < list_size(keys); i++){
		if(list_get(keys,i) == key)
			return TRUE;
	}
	return FALSE;
}

/*
 * allInserts = lista ,ordenada por tsmp, de "tsmp;key;value"
 * keys = lista de uint16_t con las todas las key a guardar
 */
void com_saveInPartition(t_list *keys,t_list *allInserts, activeTable *table)
{
	uint16_t key;
	char *tableUrl = makeTableUrl(table->name);
	char *partUrl,*part,*toInsert;


	bool _gotkey(void *insert){ //wrapper
		return com_gotKey(key,(char*)insert);
	}

	for(int i = 0; i < list_size(keys); i++){
		key = list_get(keys,i);
		toInsert = list_find(allInserts,_gotKey);

		partUrl = string_duplicate(tableUrl);
		part = string_itoa(key % table->parts);
		string_append(&partUrl,part);
		string_append(&partUrl,".bin");

		b_saveData(partUrl,toInsert);

		free(partUrl);
	}
}

bool com_gotKey(uint16_t key, char *insert)
{
	return com_key(insert) == key;
}
