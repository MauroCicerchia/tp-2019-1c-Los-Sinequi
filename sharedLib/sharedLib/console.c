#include"console.h"

e_query queryError() {
	printf("La consulta no es valida.\n");
	return QUERY_ERROR;
}

int validateQuerySyntax(char **array,e_query queryType){
	int tamano = sizeofArray(array);
	int key;
	switch(queryType){

		case QUERY_SELECT:
			if( tamano != 3) return false; // cantidad de parametros invalidos
			key = atoi(array[2]);
			if(!key) return false; //key invalida
			return true;

		case QUERY_INSERT:
			if( sizeofArray(array) != 5) return false; // cantidad de parametros invalidos
			if(atoi(array[2])) return false; //key invalida
			return true;

		case QUERY_CREATE:
			if( sizeofArray(array) != 5 ) return 0; // cantidad de parametros invalidos

			key = atoi(array[3]);
			if(!key) return false; //particiones o tiempo de compactacion invalidos

			key = atoi(array[4]);
			if(!key) return false;

			return true;

		case QUERY_DESCRIBE:
			if( sizeofArray(array) != 2 ) return false; // cantidad de parametros invalidos
			return true;

		case QUERY_DROP:
			if( sizeofArray(array) != 2 ) return false; // cantidad de parametros invalidos
			return true;

		default:
			return true;
	}

}
int sizeofArray(char **array){
	int i=0;
	if(!array[i]) return 0;
	while(array[i] != NULL){
		i++;
	}
	return i;
}
