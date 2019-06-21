#include"blocks.h"


//le asigna a cada partion de la tabla un size y un bloque vacio
void b_loadPartitionsFiles(char *tableUrl,int parts)
{
	char *url;
	char *strPart;
	for(int i = 0; i < parts; i++){
		url = string_new();
		string_append(&url,tableUrl);
		strPart = string_itoa(i);
		string_append(&url,strPart);
		string_append(&url,".bin");

		b_assignSizeAndBlock(url,0);

		free(url);
		free(strPart);
	}
}


//le asigna un bloque y el size a esa url
void b_assignSizeAndBlock(char *partUrl,int size)
{
	int blockNumber = ba_getNewBlock();

	startPartition(partUrl,blockNumber,size);
}


//trae todas los inserts de esa url, que es la particion.bin o el .tmp
t_list *b_getListOfInserts(char *partUrl)
{
	FILE *f;
	int fSize;
	t_list *listOfInserts;

	char** blocks = string_get_string_as_array(getListOfBlocks(partUrl));// ["1","43","550"]
	int size = sizeofArray(blocks); // tamano de array de bloques

	char *inserts = string_new();
	char *blockUrl; // url de cada block particular
	char *url = fs_getBlocksUrl(); //url absoluta de donde estan los bloques "mnt/blocks"
	char *pivot;
	struct stat st;
	for(int i = 0; i<size; i++)
	{
		blockUrl = string_new();
		string_append(&blockUrl,url);
		string_append(&blockUrl,blocks[i]);
		string_append(&blockUrl,".bin");


		stat(blockUrl,&st);
		fSize = st.st_size;

		pivot = malloc(fSize);

		f = fopen(blockUrl,"r");
		fread(pivot,fSize,1,f);
		fclose(f);
		pivot[fSize] = '\0';

		if(strcmp(pivot,"&")) //si no es igual a "&" lo agrego a la lista de inserts
			string_append(&inserts,pivot);

		free(pivot);
		free(blockUrl);
	}

	listOfInserts = insertsToList(inserts); //parsea el char *inserts por \n y los mete en la lista
	free(inserts); free(url); free(blocks);
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
void startPartition(char *url, int blockNumber, int size)
{
	char *toSave = string_new();

	string_append(&toSave,"SIZE=");
	char *strSize = string_itoa(size);
	string_append(&toSave,strSize);
	string_append(&toSave,"\n");
	string_append(&toSave,"BLOCKS=[");
	char *strblockNumber = string_itoa(blockNumber);
	string_append(&toSave,strblockNumber);
	free(strblockNumber);
	string_append(&toSave,"]\n");

	FILE *f = txt_open_for_append(url);
	txt_write_in_file(f,toSave);
	txt_close_file(f);
	free(toSave);
	free(strSize);
}

//toma el array de bloques del ,archivo pasado por url, asociado a BLOCKS=
char *getListOfBlocks(char *partUrl)
{
	t_config *partition = config_create(partUrl);
	char *pivot = config_get_string_value(partition,"BLOCKS");
	char *blocks = string_duplicate(pivot);
	config_destroy(partition);
	return blocks;
}


//pisa el valor de BLOCKS por el de listBlocks
void b_modifyBlocks(char *partUrl, char *listBlocks){
	t_config *partition = config_create(partUrl);
	config_set_value(partition,"BLOCKS",listBlocks);
	config_save(partition);
	config_destroy(partition);

}

int getSizeOfBlocks(){
	return metadataSizeBlocks;
}


//tb_getBlockInserts(block) revisar si es necesaria y donde


//guarda la data en el archivo de la url
void b_saveData(char *url,char *data){
 	char *blocksDirectory = fs_getBlocksUrl();
	char *blockUrl;
	int sizeOfSemiCompleteBlock = b_freeSizeOfLastBlock(url);

//	if(b_full(block)){
//		b_addNewBlock(url);
//		block = b_get_lastBlock(url);
//	}

	//en el ultimo bloque -NO- hay espacio para guardar toda la info
	if(! (sizeOfSemiCompleteBlock >= strlen(data))){
		int blocksNeeded;
		if( ((strlen(data)-sizeOfSemiCompleteBlock) % getSizeOfBlocks()) == 0 ){
			//si entra justo le doy los bloques justos
			blocksNeeded = (strlen(data)-sizeOfSemiCompleteBlock) / getSizeOfBlocks();
		}else{
			//si no entra justo le doy un bloque demas
			blocksNeeded = (strlen(data)-sizeOfSemiCompleteBlock) / getSizeOfBlocks() + 1;
		}

		//le asigno todos los bloques que necesita
		for(int i =0; i < blocksNeeded; i++){b_addNewBlock(url);}

	}
	//en el ultimo bloque hay espacio suficiente para guardar la info completa
	else{
		blockUrl = string_new();
		string_append(&blockUrl, blocksDirectory);
		string_append(&blockUrl, string_itoa(b_get_lastBlock(url)));
		string_append(&blockUrl, ".bin");
		b_saveIntoBlock(blockUrl,data);
		free(blockUrl);
		free(blocksDirectory);
		return;
	}
	//aca llega solo si no entro en el "else" de arriba
	int flag = 1;
	int lastPosInserted = 0;

	blockUrl = string_new();
	string_append(&blockUrl, blocksDirectory);
	string_append(&blockUrl, string_itoa(b_get_firstFreeBlock(url)));
	string_append(&blockUrl, ".bin");

	//lleno el bloque que estaba semicompleto
	char *toInsert = string_substring(data, lastPosInserted, sizeOfSemiCompleteBlock);
	b_saveIntoBlock(blockUrl, toInsert);
	free(toInsert);
	free(blockUrl);

	lastPosInserted = sizeOfSemiCompleteBlock;

	while(flag){
		toInsert = string_new();
		if( (strlen(data)-lastPosInserted) > getSizeOfBlocks() ){ //si lo que queda no entra en un bloque
			toInsert = string_substring_from(data, lastPosInserted);

			blockUrl = string_new();
			string_append(&blockUrl, blocksDirectory);
			string_append(&blockUrl, string_itoa(b_get_firstFreeBlock(url)));
			string_append(&blockUrl, ".bin");

			b_saveIntoBlock(blockUrl,toInsert);

			lastPosInserted += getSizeOfBlocks();

			free(toInsert); free(blockUrl);
		}
		else{// si lo que queda entra en un bloque
			toInsert = string_substring_from(data, lastPosInserted);

			blockUrl = string_new();
			string_append(&blockUrl, blocksDirectory);
			string_append(&blockUrl, string_itoa(b_get_firstFreeBlock(url)));
			string_append(&blockUrl, ".bin");

			b_saveIntoBlock(blockUrl, toInsert);

			free(blockUrl); free(toInsert);
			flag = 0; //condicion de corte, porque no queda mas nada que agregar
		}//else

	}//while

free(blocksDirectory);
}


//espacio libre del primer bloque no lleno
int freeSizeOfTheFirstNotFullBlock(char *url){
	return b_freeSize(b_get_firstFreeBlock(url));
}


//devuelve el ultimo bloque, osea el que tiene espacio probablemente
int b_get_lastBlock(char *url){
	char **blocksArray = string_get_string_as_array(getListOfBlocks(url));
	int last = sizeofArray(blocksArray) - 1;
	char *lastBlock = string_duplicate(blocksArray[last]);
	int x = strtol(lastBlock,NULL,10);
	free(lastBlock);
	free(blocksArray);
	return x;
}


//no deberia salir del for, osea tiene que tener siempre asignado un bloque libre al menos
//-1 si sale del for, pero nodeberia pasar
//te devuelve el primer bloque libre de la tabla
int b_get_firstFreeBlock(char *url){
	char **blocksArray = string_get_string_as_array(getListOfBlocks(url));
	for(int i = 0; i < sizeofArray(blocksArray); i++){
		if(!b_full(strtol(blocksArray[i],NULL,10)))
			return strtol(blocksArray[i],NULL,10);
	}
	return -1;
}


//dice si no queda lugar en el bloque
bool b_full(int block){
	return b_freeSize(block) == 0;
}

bool b_empty(int block){
	return b_realFreeSize(block) == getSizeOfBlocks();
}


//al archivo le agrego un nuevo bloque a la lista de bloques
void b_addNewBlock(char *url){
	int newBlock = ba_getNewBlock();
	char **listBlocks = string_get_string_as_array(getListOfBlocks(url));
	int size = sizeofArray(listBlocks);
	listBlocks[size] = string_itoa(newBlock);
	listBlocks[size + 1] = NULL;
	char *stringArray = string_new();

	string_append(&stringArray, "[");
	int i = 0;
	while(listBlocks[i] != NULL){
		string_append(&stringArray,listBlocks[i]);
		if(listBlocks[i + 1] != NULL) string_append(&stringArray,",");
		i++;
	}

	string_append(&stringArray, "]");
	b_modifyBlocks(url,stringArray);
	free(listBlocks);
	free(stringArray);
}


int b_realFreeSize(int block){
	char *url = fs_getBlocksUrl();
		char *strBlock = string_itoa(block);
		string_append(&url,strBlock);
		string_append(&url,".bin");

		struct stat st;
		stat(url,&st);

		int actualSize = st.st_size;

		free(url);
		free(strBlock);
		return (getSizeOfBlocks() - actualSize);

}

//cuantos bytes le quedan al bloque par asignar
int b_freeSize(int block){
	char *url = fs_getBlocksUrl();
	char *strBlock = string_itoa(block);
	string_append(&url,strBlock);
	string_append(&url,".bin");

	struct stat st;
	stat(url,&st);

	int actualSize = st.st_size;

	if(actualSize == 1){ //me fijo si lo que tiene es "&"
		FILE *f = fopen(url,"r");
		char *pivot = malloc(3);
		fread(pivot,1,1,f);
		fclose(f);
		pivot[actualSize] = '\0';
		if(!strcmp(pivot,"&"))
			actualSize = 0;
		free(pivot);
	}

	free(url);
	free(strBlock);
	return (getSizeOfBlocks() - actualSize);
}


//agarrar el ultimo del bloque y fijarse cuando espacio le queda
int b_freeSizeOfLastBlock(char *url){
	int lb = b_get_lastBlock(url);
	return b_freeSize(lb);
}


//guarda en la url del bloque lo que se le pasa por parametro
//esta funcion no deberia romper nunca por overflow de tamano de bloque porquese cheuquea antes de usarla
void b_saveIntoBlock(char *blockUrl,char *data)
{
	char *pivot = malloc(3);
	struct stat st;
	FILE *f;
	stat(blockUrl,&st);
	int fSize = st.st_size;

	if(fSize == 1){
		f = fopen(blockUrl,"r");
		fread(pivot,fSize,1,f);
		fclose(f);
		pivot[fSize] = '\0';
		if(!strcmp(pivot,"&")){
			f = fopen(blockUrl,"w");
			fclose(f);
		}
//		free(pivot);
	}

	FILE *ff = txt_open_for_append(blockUrl);
	txt_write_in_file(ff, data);
	txt_close_file(ff);
}

//actualiza el tamano
void b_updateSize(char *url){
	char *x = getListOfBlocks(url);
	char **blocks = string_get_string_as_array(x);
	free(x);

	int tam = (sizeofArray(blocks) - 1) * getSizeOfBlocks(); //no deberia llegarle nunca un array tam 0
	tam += (getSizeOfBlocks() - b_freeSizeOfLastBlock(url) );

	b_modifySize(url,tam);
	free(blocks);
}

void b_modifySize(char *url,int tam){
	t_config *conf = config_create(url);
	char *strtam = string_itoa(tam);
	config_set_value(conf,"SIZE",strtam);
	config_save(conf);
	config_destroy(conf);
	free(strtam);
}

//le escribe "&" para saber que tiene algo y que esta incializado
void b_writeBlockAssigned(int block)
{
	char *url = fs_getBlocksUrl();
	char *strBlock = string_itoa(block);
	string_append(&url,strBlock);
	string_append(&url,".bin");

	FILE *f = txt_open_for_append(url);
	txt_write_in_file(f,"&");
	txt_close_file(f);

	free(strBlock);
}
