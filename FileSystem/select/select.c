#include"select.h"

char *qselect(char *table, uint16_t key){

	char *url = string_new();
	string_append(&url,"tables/");
	string_append(&url,table);
	string_append(&url, ".bin");
	FILE *file = fopen(url,"r");
	free(url);
	t_list *list = list_create();

	loadList(list,file);
	txt_close_file(file);
	t_list *dataList = listToDATAmode(list);
	list_destroy(list);
	char *value = string_new();
	value = getValue(dataList,key);
	list_destroy(dataList);
	return value;
}

//carga la lista con la info del archivo
void loadList(t_list *list,FILE *file){
	char *line = string_new();
	while(!feof(file)){
		fgets(line,100,file);
		list_add(list,line);
	}
	free(line);
}

//duelvue el ultimo valor de la lista que matchea con la key
char *getValue(t_list *list,uint16_t key){
	t_list *pivot = malloc(sizeof(t_list) * list_size(list));
	pivot = list_sorted(list,biggerTimeStamp);

	bool _lastKey(void *elem){
		return isLastKey(key,elem);
	}

	dataSelect *returnValue =  (dataSelect*) list_find(pivot, _lastKey);
	free(pivot);
	return (*returnValue).value;
}

//duelvue true si el priemr elemento fue agregado despues que el segundo
bool biggerTimeStamp(dataSelect elem1, dataSelect elem2){
	return (elem1.timeStamp > elem2.timeStamp);
}

//devuelve true si el elemento que se le pasa coincide con la key
bool isLastKey(uint16_t key,void* elem){
	dataSelect *pivot = malloc(sizeof(dataSelect));
	pivot =(dataSelect*) elem;
	if(key == pivot->key){
		free(pivot);
		return true;
	}
	else {
		free(pivot);
		return false;
	}
}


//recibe una lista de "timestamp;key;value" y devuelve una transformada a tipo dataSelect
t_list *listToDATAmode(t_list *list){
	t_list *pivot = list_create();
	pivot = list_map(list,elemToDATAmode);
	return pivot;
}

//recibe un elemento de tipo "timestamp;key;value" devuelve uno de tipo dataSelect con los campos asignados respectivamente
void *elemToDATAmode(void *lfsElem){
	char **lfsArray =string_split(lfsElem, ";");
	dataSelect *pivot = malloc(sizeof(dataSelect));
	int a = atoi(lfsArray[0]);
	pivot->timeStamp = a;
	a = atoi(lfsArray[1]);
	pivot->key = a;
	pivot->value = lfsArray[2];
	return pivot;
}
