#include "describe.h"

metadata *qdescribe(char *table)
{
	if(!fs_tableExists(table)){
		log_error(logger,"No existe la tabla sobre la cual se quiere hacer describe");
		return NULL;
	}

	metadata *tableMetadata = fs_getTableMetadata(table);
	return tableMetadata;
}
