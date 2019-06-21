#include"insert.h"

int qinsert(char *table, char* key, char *value, char* timeStamp){
	log_info(logger, "  Chequeo que la tabla este creada");
	if(fs_tableExists(table)){
		log_info(logger, "  La tabla existe");
		sem_wait(&MUTEX_MEMTABLE);
		mt_insert(table,timeStamp,key,value);
		sem_post(&MUTEX_MEMTABLE);
		log_info(logger, ">>>");
		log_info(logger, "Insert Exitoso");
		log_info(logger, ">>>");
		return 1;
	}

	else log_error(logger, "La tabla sobre la que se quiere hacer el insert no existe");
	return 0;
	dump();
}

