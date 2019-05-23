typedef enum {
	QUERY_SELECT,
	QUERY_INSERT,
	QUERY_CREATE,
	QUERY_DESCRIBE,
	QUERY_DROP,
	QUERY_JOURNAL,
	QUERY_ADD,
	QUERY_RUN,
	QUERY_METRICS,
	QUERY_ERROR
}e_query;

typedef enum {
	SC,
	SHC,
	EC
}e_cons_type;

e_query getQueryType(char*);
e_cons_type getConsistencyType(char *);
