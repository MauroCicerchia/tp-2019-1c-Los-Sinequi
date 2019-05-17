#include "consistency.h"

e_cons_type getConsistencyType(char *query) {

	if (!strcasecmp("SC", query))
		return SC;
	else if (!strcasecmp("SHC", query))
		return SHC;
	else if (!strcasecmp("EC", query))
			return EC;
	return CONS_ERROR;
}
