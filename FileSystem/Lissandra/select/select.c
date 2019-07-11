#include"select.h"

//evaluar el caso de si no se hizo ningun insert
char *qselect(char *table, char* strKey){
	delayer();

	uint16_t key = strtol(strKey,NULL,10);

	//verificamos que exista la tabla
	if (!fs_tableExists(table)){
		log_error(logger, "[SELECT]: No existe la tabla sobre la cual se intenta hacer el SELECT");
		return NULL; //no existe la tabla
	}

	log_info(logger, "[SELECT]: Leyendo info de bloques a memoria..");

	t_list *list = fs_getListOfInserts(table,key);
	log_info(logger, "[SELECT]: Leidos!");

	//caso que no se hizo ningun INSERT
	if(list_size(list) == 0){
		log_error(logger, "[SELECT]: No se hizo ningun insert");
		list_destroy(list);
		return NULL;
	}

	t_list *dataList = listToDATAmode(list);

	list_destroy_and_destroy_elements(list,free); //limpio y destruyo lista

	log_info(logger, "[SELECT]: Leyendo el mayor valor asociado a la key...");

	char *value = getValue(dataList,key); //obtengo el ultimo valor de la datalist que matchea con la key

	list_destroy_and_destroy_elements(dataList,dataSelect_destroy); //limpio y destruyo dalaList

	return value;
}

//libero memoria de data
void dataSelect_destroy(void* data ){
	free(((dataSelect*)data)->value);
	free(data);
}

//duelvue el ultimo valor de la lista que matchea con la key
char *getValue(t_list *list,uint16_t key){
	t_list *sortedList;
	sortedList = list_sorted(list,biggerTimeStamp);

	bool _lastKey(void *elem){
		return isLastKey(key,elem);// compara si el elemento que se le pasa coincide con la key
	}

	char *value = malloc(sizeof(char)*100);
	dataSelect *returnValue =  (dataSelect*) list_find(sortedList, _lastKey);
	if(returnValue == NULL){   //en caso que no haya valores asociados a la tabla
		log_error(logger, "[SELECT]: No hay values asociados a esa Key");
		list_destroy(sortedList);
		free(value);
		return NULL;
	}
	else strcpy(value,returnValue->value); //copio valor asociado a la tabla

	list_destroy(sortedList);

	log_info(logger, "[SELECT]: Valor encontrado!");

	return value; //retorno el valor asociado a la key
}

//duelvue true si el primer elemento fue agregado despues que el segundo elemento
bool biggerTimeStamp(void *elem1, void *elem2){
	return ((dataSelect*)elem1)->timeStamp > ((dataSelect*)elem2)->timeStamp;
}

//devuelve true si el elemento que se le pasa,coincide con la key
bool isLastKey(uint16_t key,void* elem){
	return key == ((dataSelect*)elem)->key;
}


//recibe una lista de: "timestamp;key;value", devuelve una transformada a tipo dataSelect
t_list *listToDATAmode(t_list *list){
	return list_map(list,elemToDATAmode);
}

//recibe un elemento de tipo: "timestamp;key;value", devuelve uno de tipo dataSelect con los campos asignados respectivamente
void *elemToDATAmode(void *lfsElem){
	char **lfsArray = string_split(lfsElem, ";");
	dataSelect *pivot = malloc(sizeof(dataSelect)); //alojo memoria

	pivot->timeStamp = strtouq((char*)lfsArray[0],NULL,10);
	pivot->key = strtol((char*)lfsArray[1],NULL,10);
	pivot->value = string_duplicate(lfsArray[2]);

	free(lfsArray[0]);
	free(lfsArray[1]);
	free(lfsArray[2]);
	free(lfsArray);

	return pivot;
}
