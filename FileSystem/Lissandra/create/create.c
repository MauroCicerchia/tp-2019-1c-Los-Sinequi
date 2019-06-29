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
	int parts = strtol(partitions,NULL,10);
	int ctime = strtol(compactime,NULL,10);
	int flag = fs_create(table,consistency,parts,ctime);

	if(flag){
		pthread_t tNewTable;
		activeTable *createdTable = addToActiveTables(table,parts,ctime);
//		pthread_create(&tNewTable,NULL,(void*)threadCompact, string_duplicate(createdTable->name));
//		pthread_detach(tNewTable);
	}
	return flag;
}


activeTable *addToActiveTables(char *table, int parts, int ctime)
{
	activeTable *newTable = malloc(sizeof(activeTable));

	newTable->name = string_duplicate(table);
	newTable->ctime = ctime;
	newTable->parts = parts;
	sem_init(&newTable->MUTEX_DROP_TABLE,1,1);
	sem_init(&newTable->MUTEX_TABLE_PART,1,1);

	list_add(sysTables,newTable);
	return newTable;
}
