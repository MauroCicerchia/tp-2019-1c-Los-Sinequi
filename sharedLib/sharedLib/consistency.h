#include<stdio.h>
#include<commons/string.h>

#ifndef CONSISTENCY_H
#define CONSISTENCY_H

typedef enum {
	CONS_SC,
	CONS_SHC,
	CONS_EC,
	CONS_UNDEFINED,
	CONS_ERROR
} e_cons_type;

e_cons_type getConsistencyType(char*);
char *getConsistencyStr(e_cons_type);

#endif

