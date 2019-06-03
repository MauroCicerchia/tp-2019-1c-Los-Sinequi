#include"create.h"

int qcreate(char *table, char *consistency, char *partitions, char *compactime){
	if(fs_tableExists(table)){
		printf("La tabla ya existe\n");
		return 0;
	}
	if(getConsistencyType(consistency) == CONS_ERROR){
		printf("Tipo de consistencia invalido\n");
		return 0;
	}
	int parts = atoi(partitions);
	int ctime = atoi(compactime);
	return fs_create(table,consistency,parts,ctime);
}

