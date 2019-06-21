#include "drop.h"

int qdrop(char *table)
{
	if(!fs_tableExists(table)){
		log_error(logger,"La tabla que se intenta droppear no existe");
		return 0;
	}

	freeBlocks(table);

	deleteDirectoriesAndFiles(table);

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
	}
	closedir(d);

	rmdir(tableUrl); //borro la carpeta

	free(tableUrl);
}

void freeBlocks(char *table)
{

}
