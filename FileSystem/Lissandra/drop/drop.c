#include "drop.h"

int qdrop(char *table) //agregar semaforo de drop para que nadie toque las tablas meintras se dropean
{
	delayer();

	log_info(logger,"[DROP]: Chequeando que la tabla exista...");
	if(!fs_tableExists(table)){
		log_error(logger,"[DROP]: La tabla que se intenta droppear no existe");
		return 0;
	}
	log_info(logger,"[DROP]: La tabla existe!");

	activeTable *acTable = com_getActiveTable(table);

	log_info(logger,"[DROP]: Inicia borrado...");
	pthread_mutex_lock(&acTable->MUTEX_DROP_TABLE);
		log_info(logger,"[DROP]: Liberando bloques...");
		freeBlocks(table);

		log_info(logger,"[DROP]: Borrando directorios...");
		deleteDirectoriesAndFiles(table);

	pthread_mutex_unlock(&acTable->MUTEX_DROP_TABLE);

	deleteTableFromMemory(table);

	return 1;
}

void deleteDirectoriesAndFiles(char *table)
{
	char *tableUrl = makeTableUrl(table);
	char *file;

	DIR *d;
	struct  dirent *dir;

	d = opendir(tableUrl);
	dir = readdir(d);
	while(dir != NULL){ //borro todos los archivos del directorio
		if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")){
			file = string_duplicate(tableUrl);
			string_append(&file,dir->d_name);
			unlink(file);
			free(file);
		}
		dir = readdir(d);
	}
	closedir(d);

	rmdir(tableUrl); //borro la carpeta

	free(tableUrl);
}

void freeBlocks(char *table)
{
	char *tableUrl = makeTableUrl(table);
	t_list *files = getFiles(tableUrl);
	char *fileUrl, *blockUrl, *strBlocks;
	char **blocks;
	int iBlock;
	FILE *f;

	for(int i = 0; i < list_size(files); i++){ //recorro archivo por archivo
		fileUrl = string_duplicate(tableUrl);
		string_append(&fileUrl,list_get(files,i));

		strBlocks = getListOfBlocks(fileUrl);
		blocks = string_get_string_as_array(strBlocks);

		for(int j = 0; j < sizeofArray(blocks); j++){ //borro bloque por bloque
			blockUrl = fs_getBlocksUrl();
			string_append(&blockUrl,blocks[j]);
			string_append(&blockUrl,".bin");
			f = fopen(blockUrl,"w"); //borro el contenido del bloque
			fclose(f);

			free(blockUrl);

			iBlock = strtol(blocks[j],NULL,10);
			ba_freeBlock(iBlock); //libero en bloque en el bitarray
		}

		free(fileUrl); free(strBlocks);
	}
	list_destroy_and_destroy_elements(files, free);

	for(int k = 0; k < sizeofArray(blocks);k++){free(blocks[k]);}
	free(blocks);
	free(tableUrl);
}

//devuelve lista con tmps y las particiones
t_list *getFiles(char *tableUrl)
{
	t_list *toReturn = list_create();

	DIR *d;
	struct  dirent *dir;

	d = opendir(tableUrl);
	dir = readdir(d);
	while(dir != NULL){
		if( strcmp(dir->d_name,".") &&
			strcmp(dir->d_name,"..")&&
			strcmp(dir->d_name,"Metadata.bin")){
				char *toadd = string_duplicate(dir->d_name);
				list_add(toReturn,toadd);
		}
		dir = readdir(d);
	}
	closedir(d);
	return toReturn;
}

//saca la tabla de la lista de tablas activas en memoria
void deleteTableFromMemory(char *table){

	void _activeTableDestroyer(void *tableToDestroy){
		free(((activeTable*)tableToDestroy)->name);
		pthread_mutex_destroy(&((activeTable*)tableToDestroy)->MUTEX_DROP_TABLE);
		pthread_mutex_destroy(&((activeTable*)tableToDestroy)->MUTEX_TABLE_PART);
		free(tableToDestroy);
	}

	activeTable *pivot;
	for(int i = 0; i < list_size(sysTables); i++){
		pivot = list_get(sysTables,i);
		if(!strcmp(table,pivot->name)){
			list_remove_and_destroy_element(sysTables,i, _activeTableDestroyer);
			return;
		}
	}
}
