#include"console.h"

e_query getQueryType(char *query) {

	if (!strcasecmp("SELECT", query))
		return QUERY_SELECT;
	else if (!strcasecmp("INSERT", query))
		return QUERY_INSERT;
	else if (!strcasecmp("CREATE", query))
			return QUERY_CREATE;
	else if (!strcasecmp("DESCRIBE", query))
			return QUERY_DESCRIBE;
	else if (!strcasecmp("DROP", query))
			return QUERY_DROP;
	else if (!strcasecmp("JOURNAL", query))
			return QUERY_JOURNAL;
	else if (!strcasecmp("ADD", query))
			return QUERY_ADD;
	else if (!strcasecmp("RUN", query))
			return QUERY_RUN;
	else if (!strcasecmp("METRICS", query))
			return QUERY_METRICS;

	return QUERY_ERROR;


}

e_query queryError() {
	printf("La consulta no es valida.\n");
	return QUERY_ERROR;
}
