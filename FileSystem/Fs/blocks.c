#include"blocks.h"


//le asigna a cada partion de la tabla un size y un bloque vacio
void b_loadPartitionsFiles(char *tableUrl,int parts)
{
	int blockNumber;
	for(int i = 0; i < parts; i++){
		string_append(&tableUrl,i);
		string_append(&tableUrl,".bin");

		assignSizeAndBlock(tableUrl);
	}
}


//le asigna un bloque y el size a esa url
void b_assignSizeAndBlock(char *partUrl)
{
	int blockNumber = ba_getNewBlock();

	startPartition(partUrl,blockNumber);
}


//trae todas los inserts de esa url, que es la particion.bin o el .tmp
t_list *b_getListOfInserts(char *partUrl)
{
	t_list *listOfInserts = list_create();
	t_list *pivot;

	char** inserts = string_get_string_as_array(getListOfBlocks(partUrl));

	int block;
	for(int i = 0; i<sizeofArray(inserts); i++)
	{
		block = strtol(inserts[i],NULL,10);
		pivot = b_getListOfInserts(block);
		list_add_all(listOfInserts, pivot);
		free(pivot);
	}

	return listOfInserts;
}


//devuelve un boque libre y lo marca como usado en el bitarray
int ba_getNewBlock()
{

}


//abre la particion de la url y le carga un bloque inicial y el size
void startPartition(char *url, int blockNumber)
{
	char *toSave = string_new();
	int newBlock = ba_getNewBlock();

	string_append(&toSave,"SIZE=");
	//agregar size
	string_append(&toSave,"\n");
	string_append(&toSave,"BLOCKS=[");
	string_append(&toSave,string_itoa(blockNumber));
	string_append(&toSave,"]\n");

	FILE *f = txt_open_for_append(url);
	txt_write_in_file(f,toSave);
	txt_close_file(f);
}

//toma el array de bloques del ,archivo pasado por url, asociado a BLOCKS=
char *getListOfBlocks(partUrl)
{
	t_config *partition = config_create(partUrl);
	char *blocks = config_get_string_value(partition,"BLOCKS");
	config_destroy(partition);
	return blocks;
}


/*DEPRECATED*/
//duelve una lista con la info del archivo
t_list *b_getListOfInserts(char* table,int key){
	char *partition = string_itoa(key % strtol(getPartitions(),NULL,10));
	FILE *file;
	char *url = makeTableUrl(table);
	string_append(&url,partition);
	string_append(&url, ".bin");
	t_list *list = b_getListOfInserts(url); //trae todas los inserts de esa url, que es la particion.bin
	free(url);
	return list;
}



//void b_loadPartitionsFiles(int parts);   //le asigna el size y un bloque a cada bloque de particion
//void b_assignSizeAndBlock(tableUrl) ;  //le asigna un bloque y el size a esa url
// t_list *b_getListOfInserts(url); //trae todas los inserts de esa url, que es la particion.bin

