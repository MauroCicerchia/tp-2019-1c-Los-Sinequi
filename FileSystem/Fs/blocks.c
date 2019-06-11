#include"blocks.h"


//le asigna a cada partion de la tabla un size y un bloque vacio
void b_loadPartitionsFiles(char *tableUrl,int parts)
{
	int blockNumber;
	for(int i = 0; i < parts; i++){
		string_append(&tableUrl,i);
		string_append(&tableUrl,".bin");

		b_assignSizeAndBlock(tableUrl);
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
	FILE *f;
	long fSize;
	t_list *listOfInserts;

	char** blocks = string_get_string_as_array(getListOfBlocks(partUrl));// ["1","43","550"]
	int size = sizeofArray(blocks); // tamano de array de bloques

	char *inserts = string_new();
	char *blockUrl; // url de cada block particular
	char *url = fs_getBlocksUrl(); //url absoluta de donde estan los bloques "mnt/blocks"
	char *pivot;
	for(int i = 0; i<size; i++)
	{
		pivot = (char*)malloc(getSizeOfBlocks(partUrl));

		blockUrl = string_new();
		string_append(&blockUrl,url);
		string_append(&blockUrl,blocks[i]);

		f = fopen(blockUrl,"r");
		fseek(f,0,SEEK_END);
		fSize = ftell(f);
		fseek(f,0,SEEK_SET);
		fread(pivot,1,fSize,f);
		fclose(f);

		string_append(&inserts,pivot);

		free(pivot); free(blockUrl);
	}

	listOfInserts = insertsToList(inserts); //parsea el char *inserts por \n y los mete en la lista
	free(inserts); free(url);
	return listOfInserts;
}

t_list *insertsToList(char *inserts)
{
	t_list *toReturn = list_create();
	char **arrayOfInserts = string_split(inserts,"\n");
	char *pivot;
	for(int i =0; i<sizeofArray(arrayOfInserts); i++)
	{
		pivot = string_duplicate(arrayOfInserts[i]);
		list_add(toReturn,pivot);
	}
	free(arrayOfInserts);
	return toReturn;
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
char *getListOfBlocks(char *partUrl)
{
	t_config *partition = config_create(partUrl);
	char *blocks = config_get_string_value(partition,"BLOCKS");
	config_destroy(partition);
	return blocks;
}

int getSizeOfBlocks(){
	return config_get_int_value(lfsMetadata,"BLOCK_SIZE");
}


tb_getBlockInserts(block)
{

}

//
char **getAllTmps(char *tableUrl)
{

}



void b_saveData(char *url,char *data){

}
//void b_loadPartitionsFiles(int parts);   //le asigna el size y un bloque a cada bloque de particion
//void b_assignSizeAndBlock(tableUrl) ;  //le asigna un bloque y el size a esa url
// t_list *b_getListOfInserts(url); //trae todas los inserts de esa url, que es la particion.bin

