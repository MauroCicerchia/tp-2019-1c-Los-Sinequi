#include"blocks.h"


//le asigna a cada partion de la tabla un size y un bloque vacio
void b_loadPartitionsFiles(char *tableUrl,int parts)
{
	for(int i = 0; i < parts; i++){
		string_append(&tableUrl,string_itoa(i));
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


//abre la particion de la url y le carga un bloque inicial y el size
void startPartition(char *url, int blockNumber)
{
	char *toSave = string_new();
//	int newBlock = ba_getNewBlock();

	string_append(&toSave,"SIZE=");
	//agregar size
	string_append(&toSave,"\n");
	string_append(&toSave,"BLOCKS=[");
	string_append(&toSave,string_itoa(blockNumber));
	string_append(&toSave,"]\n");

	FILE *f = txt_open_for_append(url);
	txt_write_in_file(f,toSave);
	txt_close_file(f);
	free(toSave);
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


//tb_getBlockInserts(block) revisar si es necesaria y donde


//typedef struct{
//	char * table; //nombre de table
//	int freeBlock; //el ultimo bloque, osea el que tiene espacio
//	int freeSize; //espacio en bytes del ultimo bloque libre
//}tableInfo;

//guarda la data en el archivo de la url
void b_saveData(char *url,char *data){   //"HOLACOMOESTASTODOBIEN"  //HOLA COMOESTASTO DOBIEN
	const char *blocksDirectory = fs_getBlocksUrl();
	char *blockUrl;
	int sizeOfSemiCompleteBlock = b_freeSizeOfLastBlock(url);
//	if(b_full(block)){
//		b_addNewBlock(url);
//		block = b_get_lastBlock(url);
//	}

	//en el ultimo bloque no hay espacio para guardar toda la info
	if(! sizeOfSemiCompleteBlock >= strlen(data)){
		int blocksNeeded;
		if( ((strlen(data)-sizeOfSemiCompleteBlock) % getSizeOfBlocks()) == 0 ){
			//si entra justo le doy el tam justo
			blocksNeeded = (strlen(data)-sizeOfSemiCompleteBlock) / getSizeOfBlocks();
		}else{
			//si no entra justo le doy un bloque demas
			blocksNeeded = (strlen(data)-sizeOfSemiCompleteBlock) / getSizeOfBlocks() + 1;
		}

		//le asigno todos los bloques que necesita
		for(int i =0; i < blocksNeeded+1; i++){b_addNewBlock(url);}

	}
	//en el ultimo bloque hay espacio suficiente para guardar la info completa
	else{
		blockUrl = string_new();
		string_append(&blockUrl, blocksDirectory);
		string_append(&blockUrl, string_itoa(b_get_lastBlock(url)));
		string_append(&blockUrl, ".bin");
		b_saveIntoBlock(blockUrl,data);
		free(blockUrl); free(blocksDirectory);
		return;
	}
	//aca llega solo si no entro en el "else" de arriba
	int flag = 1;
	int lastPosInserted = 0;
	while(flag){
		string_substring(data,lastPosInserted, freeSizeOfTheFirstNotFullBlock(url));
	}


	int block = b_get_lastBlock(url);


	int freeSizeB = b_freeSize(block);
	int insertedData = 0;
//	int flag = 0; //corte del while
	char *blockUrl = string_new();
	string_append(&blockUrl, blocksDirectory);
	string_append(&blockUrl, string_itoa(b_get_lastBlock(url)));
	string_append(&blockUrl, ".bin");
	b_saveIntoBlock(blockUrl,data);


	//lleno el bloque que estaba semicompleto
	char *toInsert = string_substring(data, insertedData, sizeOfSemiCompleteBlock);
	b_saveIntoBlock(blockUrl, toInsert);
	free(toInsert); free(blockUrl);
	insertedData = sizeOfSemiCompleteBlock;

	while(){
		toInsert = string_new();
		if( (strlen(data)-insertedData) > getSizeOfBlocks() ){ //si lo que queda no entra en un bloque
			toInsert = string_substring(data, insertedData,getSizeOfBlocks());

			blockUrl = string_new();
			string_append(&blockUrl, blocksDirectory);
			string_append(&blockUrl, string_itoa(b_get_firstFreeBlock(url)));
			string_append(&blockUrl, ".bin");

			b_saveIntoBlock(blockUrl,toInsert);

			insertedData += getSizeOfBlocks();

			free(toInsert); free(blockUrl);
		}
		else{
			toInsert = string_substring_from(data, insertedData);

			blockUrl = string_new();
			string_append(&blockUrl, blocksDirectory);
			string_append(&blockUrl, string_itoa(b_get_firstFreeBlock(url)));
			string_append(&blockUrl, ".bin");

			b_saveIntoBlock(blockUrl, data);

			free(blockUrl); free(toInsert);
			flag = 0; //condicion de corte, porque no queda mas nada que agregar
		}//else

	}//while
}


//espacio libre del primer bloque no lleno
int freeSizeOfTheFirstNotFullBlock(char *url){
	return b_freeSize(b_get_firstFreeBlock(url));
}


//devuelve el ultimo bloque, osea el que tiene espacio probablemente
int b_get_lastBlock(char *url){
	char **blocksArray = string_get_string_as_array(getListOfBlocks(url));
	int last = sizeofArray(blocksArray) - 1;
	return blocksArray[last];
}


//no deberia salir del for, osea tiene que tener siempre asignado un bloque libre al menos
//-1 si sale del for, pero nodeberia pasar
//te devuelve el primer bloque libre de la tabla
int b_get_firstFreeBlock(url){
	char **blocksArray = string_get_string_as_array(getListOfBlocks(url));
	for(int i = 0; i < sizeofArray(blocksArray); i++){
		if(!b_full(string_itoa(blocksArray[i]))) return string_itoa(blocksArray[i]);
	}
	return -1;
}


//dice si no queda lugar en el bloque
bool b_full(int block){
	int blockSize = getSizeOfBlocks();

}

//al archivo le agrego un nuevo bloque a la lista de bloques
void b_addNewBlock(char *url){
	int newBlock = ba_getNewBlock();

}

//cuantos bytes le quedan al bloque par asignar
int b_freeSize(int block)

//agarrar el ultimo del bloque y fijarse cuando espacio le queda
char *freeSizeOfLastBlock(url)


//guarda en la url del bloque lo que se le pasa por parametro
//esta funcion no deberia romper nunca por overflow de tamano de bloque porquese cheuquea antes de usarla
b_saveIntoBlock(blockUrl,data);
