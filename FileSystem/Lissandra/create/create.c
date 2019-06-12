#include"create.h"

int qcreate(char *table, char *consistency, char *partitions, char *compactime){
	log_info(logger, "  Chequeo si la tabla ya existe en el FS");
	if(fs_tableExists(table)){
		log_error(logger,"La tabla ya existe");
		return 0;
	}
	log_info(logger, "  Chequeo que el tipo de consistencia sea valido");
	if(getConsistencyType(consistency) == CONS_ERROR){
		log_error(logger,"Tipo de consistencia invalido");
		return 0;
	}
	int parts = atoi(partitions);
	int ctime = atoi(compactime);
	return fs_create(table,consistency,parts,ctime);
}

