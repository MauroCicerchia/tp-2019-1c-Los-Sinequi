#include"insert.h"

int qinsert(char *table, char* key, char *value, char* timeStamp){
	delayer();

	if(strlen(value) > valueSize){ //el valor a insertar supera la longitud ya definida
		log_error(logger,"[INSERT]: El value supera la longitud definida");

		return 0;
	}

	log_info(logger, "[INSERT]: Chequeando existencia de la tabla..");

	//si el valor no supera, chequeo la existencia de la tabla sobre la que hacemos el insert
	if(!fs_tableExists(table)){//la tabla no existe
		log_error(logger, "[INSERT]: La tabla sobre la que se quiere hacer el insert no existe");

		return 0;
	} //si existe la tabla
	else log_info(logger, "[INSERT]: La tabla existe");

		log_info(logger,"[INSERT]: Insertando en memtable..");

		//inserto en la memetable
		pthread_mutex_lock(&MUTEX_MEMTABLE); //bloqueo memtable mientras hace la asignacion
		mt_insert(table,timeStamp,key,value);
		pthread_mutex_unlock(&MUTEX_MEMTABLE);

		log_info(logger, "[INSERT]: Insert Exitoso");

//		dump();
		return 1;
	}



