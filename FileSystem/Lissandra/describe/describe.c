#include "describe.h"

metadata *qdescribe(char *table)
{
	delayer();

	log_info(logger, "[DESCRIBE]: Chequeando existencia de la tabla...");

	//verifico la existencia de la tabla
	if(!fs_tableExists(table)){
		log_error(logger,"[DESCRIBE]: No existe la tabla sobre la cual se quiere hacer describe");

		return NULL; //no existe la tabla
	}

	//existe la tabla
	log_info(logger, "[DESCRIBE]: La tabla existe");

	log_info(logger, "[DESCRIBE]: Leyendo info..");
	//leo la info de metadata de la tabla
	metadata *tableMetadata = fs_getTableMetadata(table);

	if(tableMetadata != NULL) //verificamos la lectura de la info
		log_info(logger, "[DESCRIBE]: Info leida con exito");

	return tableMetadata; //devuelvo la metadata de la tabla pedida
}
