#include"create.h"

int qcreate(char *table, char *consistency, char *partitions, char *compactime){
	if(fs_tableExists(table)) return 0;
	if(!validConsistencyType(consistency)) return 0;
	int *parts = atoi(partitions);
	int *ctime = atoi(compactime);
	return fs_create(table,consistency,parts,ctime);
}


//bool fs_tableExists(char*);
//fs_create(char*,char*,int,char*);
