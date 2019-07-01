#include"insert.h"

int qinsert(char *table, char* key, char *value, char* timeStamp){
	delayer();

	if(strlen(value) > valueSize){
		log_error(logger,"[INSERT]: El value supera la longitud definida");
		return 0;
	}

	log_info(logger, "[INSERT]: Chequeando existencia de la tabla..");

	if(!fs_tableExists(table)){
		log_error(logger, "[INSERT]: La tabla sobre la que se quiere hacer el insert no existe");
		return 0;
	}else log_info(logger, "[INSERT]: La tabla existe");

		log_info(logger,"[INSERT]: Insertando en memtable..");
		sem_wait(&MUTEX_MEMTABLE);
		mt_insert(table,timeStamp,key,value);
		sem_post(&MUTEX_MEMTABLE);
		log_info(logger, "[INSERT]: Insert Exitoso");
//		dump();
		return 1;
	}



