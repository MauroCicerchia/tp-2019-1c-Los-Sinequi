#include "describe.h"

metadata *qdescribe(char *table)
{
	delayer();
	log_info(logger, "[DESCRIBE]: Chequeando existencia de la tabla...");
	if(!fs_tableExists(table)){
		log_error(logger,"[DESCRIBE]: No existe la tabla sobre la cual se quiere hacer describe");
		return NULL;
	}
	log_info(logger, "[DESCRIBE]: La tabla existe");

	log_info(logger, "[DESCRIBE]: Leyendo info..");
	metadata *tableMetadata = fs_getTableMetadata(table);

	if(tableMetadata != NULL)
		log_info(logger, "[DESCRIBE]: Info leida con exito");

	return tableMetadata;
}
