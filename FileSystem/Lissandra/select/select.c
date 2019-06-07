#include"select.h"

char *qselect(char *table, char* strKey){
	t_list *list = list_create();
	if (!fs_tableExists(table)){
		log_error(logger, "No existe la tabla sobre la cual se intenta hacer el SELECT");
		return NULL;
	}
	char *url = makeTableUrl(table);
	string_append(&url,table);
	string_append(&url, ".bin");
	if(access(url,F_OK) == -1){
		log_error(logger,"No se hizo ningun insert sobre la tabla");
		return NULL;
	}
	list = fs_getListOfInserts(table);
	if(list_size(list) == 0){
		log_error(logger, "No hay nada en la tabla");
		return NULL;
	}
	log_info(logger, "  Guardo en una lista toda la info de la tabla");
	t_list *dataList = listToDATAmode(list);
	log_info(logger, "  Convierto lista en estructura");
	list_destroy_and_destroy_elements(list,free);
	char *value = string_new();
	uint16_t key = atoi(strKey);
	value = getValue(dataList,key);
	log_info(logger, "  tomo el valor de la key");
//	list_destroy_and_destroy_elements(dataList,dataSelect_destroy); ESTA FUNCION ROMPE
	return value;
}

void dataSelect_destroy(void* data ){
	free(((dataSelect*)data)->value);
	free(data);
}

//duelvue el ultimo valor de la lista que matchea con la key
char *getValue(t_list *list,uint16_t key){
	t_list *sortedList;
	sortedList = list_sorted(list,biggerTimeStamp);

	bool _lastKey(void *elem){
		return isLastKey(key,elem);
	}
	char *value = malloc(sizeof(char)*100);
	dataSelect *returnValue =  (dataSelect*) list_find(sortedList, _lastKey);
	if(returnValue == NULL){
		log_error(logger,"No hay values con esa key");
		return NULL;
	}else strcpy(value,returnValue->value);

	list_destroy_and_destroy_elements(sortedList,dataSelect_destroy);
	return value;
}

//duelvue true si el priemr elemento fue agregado despues que el segundo
bool biggerTimeStamp(void *elem1, void *elem2){
	return ((dataSelect*)elem1)->timeStamp > ((dataSelect*)elem2)->timeStamp;
}

//devuelve true si el elemento que se le pasa coincide con la key
bool isLastKey(uint16_t key,void* elem){
	return key == ((dataSelect*)elem)->key;
}


//recibe una lista de "timestamp;key;value" y devuelve una transformada a tipo dataSelect
t_list *listToDATAmode(t_list *list){
	t_list *pivot = list_create();
	pivot = list_map(list,elemToDATAmode);
	return pivot;
}

//recibe un elemento de tipo "timestamp;key;value" devuelve uno de tipo dataSelect con los campos asignados respectivamente
void *elemToDATAmode(void *lfsElem){
	char **lfsArray = string_split(lfsElem, ";");
	dataSelect *pivot = malloc(sizeof(dataSelect));
	pivot->timeStamp = strtouq((char*)lfsArray[0],NULL,10);
	pivot->key = strtol((char*)lfsArray[1],NULL,10);
	pivot->value = lfsArray[2];
	return pivot;
}
