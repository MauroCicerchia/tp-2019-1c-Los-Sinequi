#include"serializacion.h"

/*
 [select,tabla,key]
	paquete = crear_paquete(querySELECT);
	addtopackage(paquete,tabla,strlen(tabla)+1);
	addtopackage(paquete,key,sizeof(int));
	sendpackage(paquete,socket);
	deletepackage(paquete);
*/

void* serializar_paquete(t_package* paquete, int bytes){

	void * serializedPackage= malloc(bytes);
	int desplazamiento = 0;

	memcpy(serializedPackage + desplazamiento, &(paquete->op_code), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(serializedPackage + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(serializedPackage + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return serializedPackage;
}

t_package* create_package(){
	t_package* paquete = malloc(sizeof(t_package));
//	paquete->op_code = queryType;
	crear_buffer(paquete);
	return paquete;
}

void create_buffer(t_package* paquete){

	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void add_to_package(t_package* paquete, void* valor, int tamanio){

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}


void send_package(t_package* paquete, int socket_cliente){

	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void delete_package(t_package* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void set_buffer(t_package *paquete,char **args){

	switch(paquete->op_code)

		case QUERY_SELECT:

			add_to_package(paquete,args[1],strlen(args[1])+1);
			add_to_package(paquete,atoi(args[2]),sizeof(int));

			break;

		case QUERY_INSERT:
			add_to_package(paquete,args[1],strlen(args[1])+1);
			add_to_package(paquete,atoi(args[2]),sizeof(int));
			add_to_package(paquete,args[3],strlen(args[3])+1);

			break;

		case QUERY_CREATE: //[nombretabla,tipoconsistencia,numeroparticiones,compactationtime]
			add_to_package(paquete,args[1],strlen(args[1])+1);
			add_to_package(paquete,getConsistencyType(args[2]),sizeof(e_cons_type));
			add_to_package(paquete,atoi(args[3]),sizeof(int));
			add_to_package(paquete,atoi(args[4]),sizeof(int));


			break;

		case QUERY_DESCRIBE: //tabla
			add_to_package(paquete,args[1],strlen(args[1])+1);

			break;

		case QUERY_DROP: //tabla
			add_to_package(paquete,args[1],strlen(args[1])+1);

			break;

		case QUERY_JOURNAL: //nada

			break;

}


