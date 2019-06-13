#include "consistency.h"

e_cons_type getConsistencyType(char *query) {

	if (!strcasecmp("SC", query))
		return CONS_SC;
	else if (!strcasecmp("SHC", query))
		return CONS_SHC;
	else if (!strcasecmp("EC", query))
			return CONS_EC;
	return CONS_ERROR;
}

char *getConsistencyStr(e_cons_type cType) {
	switch(cType) {
		case CONS_SC: return "SC";
		case CONS_SHC: return "SHC";
		case CONS_EC: return "EC";
		default: return "";
	}
}
