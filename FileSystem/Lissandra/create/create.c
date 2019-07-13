#include"create.h"

int qcreate(char *table, char *consistency, char *partitions, char *compactime){

	delayer();

	log_info(logger, "[CREATE]: Chequeando si la tabla ya existe..");
	//chequeo si la tabla existe
	if(fs_tableExists(table)){
		log_error(logger,"[CREATE]: La tabla ya existe");//en caso de existir tiro un log de error
		return 0;
	}
	//si no existe, habilito la creacion
	log_info(logger,"[CREATE]: La tabla no existe, creacion habilitada");

	//valido la consistencia
	log_info(logger, "[CREATE]: Chequeando consistencia..");
	if(getConsistencyType(consistency) == CONS_ERROR){ //si el tipo de la consistencia no es valido logueo un error
		log_error(logger,"[CREATE]: Tipo de consistencia invalido");
		return 0;
	}
	log_info(logger, "[CREATE]: Consistencia valida");

	//creo un nuevo hilo de compactacion
	int parts = strtol(partitions,NULL,10); //particiones
	int ctime = strtol(compactime,NULL,10); //tiempo
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
{   //creo la tabla y la agrego a la lista de tablas
	activeTable *newTable = malloc(sizeof(activeTable));

	newTable->name = string_duplicate(table);
	newTable->ctime = ctime;
	newTable->parts = parts;
	pthread_mutex_init(&newTable->MUTEX_DROP_TABLE,NULL);
	pthread_mutex_init(&newTable->MUTEX_TABLE_PART,NULL);

	list_add(sysTables,newTable);
	return newTable;
}
