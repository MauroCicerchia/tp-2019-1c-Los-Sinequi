#include"create.h"

int qcreate(char *table, char *consistency, char *partitions, char *compactime){

	delayer();

	log_info(logger, "[CREATE]: Chequeando si la tabla ya existe..");
	if(fs_tableExists(table)){
		log_error(logger,"[CREATE]: La tabla ya existe");
		return 0;
	}
	log_info(logger,"[CREATE]: La tabla no existe, creacion habilitada");


	log_info(logger, "[CREATE]: Chequeando consistencia..");
	if(getConsistencyType(consistency) == CONS_ERROR){
		log_error(logger,"[CREATE]: Tipo de consistencia invalido");
		return 0;
	}
	log_info(logger, "[CREATE]: Consistencia valida");

	int parts = strtol(partitions,NULL,10);
	int ctime = strtol(compactime,NULL,10);
	int flag = fs_create(table,consistency,parts,ctime);

	log_info(logger, "[CREATE]: Creando nuevo hilo de compactacion..");
	if(flag){
		pthread_t tNewTable;
		activeTable *createdTable = addToActiveTables(table,parts,ctime);
		pthread_create(&tNewTable,NULL,(void*)threadCompact, string_duplicate(createdTable->name));
		pthread_detach(tNewTable);
	}
	log_info(logger, "[CREATE]: Hilo creado y funcionando");
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
