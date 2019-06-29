#include"packaging.h"

/*
 [select,tabla,key]
	paquete = crear_paquete(querySELECT);
	addtopackage(paquete,tabla,strlen(tabla)+1);
	addtopackage(paquete,key,sizeof(int));
	sendpackage(paquete,socket);
	deletepackage(paquete);
*/

void *serialize_package(t_package *package, int bytes) {
	void *serializedPackage = malloc(bytes);
	int offset = 0;

	memcpy(serializedPackage + offset, &(package->op_code), sizeof(int));
	offset += sizeof(int);
	memcpy(serializedPackage + offset, package->buffer->stream, package->buffer->size);
	offset += package->buffer->size;

	return serializedPackage;
}

t_package *create_package(e_query queryType) {
	t_package *package = malloc(sizeof(t_package));
	package->op_code = queryType;
	create_buffer(package);
	return package;
}

void create_buffer(t_package *paquete){
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void add_to_package(t_package *package, void *value, int size) {
	package->buffer->stream = realloc(package->buffer->stream, package->buffer->size + size + sizeof(int));

	memcpy(package->buffer->stream + package->buffer->size, &size, sizeof(int));
	memcpy(package->buffer->stream + package->buffer->size + sizeof(int), value, size);

	package->buffer->size += size + sizeof(int);
}


void send_package(t_package *package, int socket_cliente) {
	int bytes = package->buffer->size + 2*sizeof(int);
	void *to_send = serialize_package(package, bytes);

	send(socket_cliente, to_send, bytes, 0);

	free(to_send);
}

void delete_package(t_package *package) {
	free(package->buffer->stream);
	free(package->buffer);
	free(package);
}

void set_buffer(t_package *paquete,char **args) {

	switch(paquete->op_code) {

		case QUERY_SELECT: //[nombretabla, key]
			add_to_package(paquete, args[1], strlen(args[1])+1);
			add_to_package(paquete, (void*)strtol(args[2],NULL,10), sizeof(int));
			break;

		case QUERY_INSERT: //[nombretabla, key, value, timestamp]
			add_to_package(paquete, args[1], strlen(args[1])+1);
			add_to_package(paquete, (void*)strtol(args[2],NULL,10), sizeof(int));
			add_to_package(paquete, args[3], strlen(args[3])+1);
			break;

		case QUERY_CREATE: //[nombretabla, tipoconsistencia, numeroparticiones, compactationtime]
			add_to_package(paquete,args[1],strlen(args[1])+1);
			add_to_package(paquete,args[2],strlen(args[1])+1);
			add_to_package(paquete,(void*)strtol(args[2],NULL,10),sizeof(int));
			add_to_package(paquete,(void*)strtol(args[2],NULL,10),sizeof(int));
			break;

		case QUERY_DESCRIBE: //[tabla]
			add_to_package(paquete, args[1], strlen(args[1])+1);
			break;

		case QUERY_DROP: //[tabla]
			add_to_package(paquete, args[1], strlen(args[1])+1);
			break;

		case QUERY_JOURNAL: //[]
			break;
	}
}

void send_int(int socket, int value) {
	int size = sizeof(value);
	send(socket, &size, sizeof(size), 0);
	send(socket, &value, size, 0);
}

void send_str(int socket, char *value) {
	int size = sizeof(char) * (strlen(value) + 1);
	send(socket, &size, sizeof(size), 0);
	send(socket, value, size, 0);
}

void send_q_type(int socket, e_query qType) {
	send(socket, &qType, sizeof(qType), 0);
}

void send_cons_type(int socket, e_cons_type cType) {
	send(socket, &cType, sizeof(cType), 0);
}

void send_req_code(int socket, e_request_code reqCode) {
	send(socket, &reqCode, sizeof(reqCode), 0);
}

void send_res_code(int socket, e_response_code resCode) {
	send(socket, &resCode, sizeof(resCode), 0);
}

int recv_int(int socket) {
	int size, value;
	recv(socket, &size, sizeof(size), 0);
	recv(socket, &value, size, 0);
	return value;
}

char *recv_str(int socket) {
	int size;
	char *value;
	recv(socket, &size, sizeof(size), 0);
	if(size == 0)
		return NULL;
	value = (char*)malloc(size);
	recv(socket, value, size, 0);
	return value;
}

e_query recv_q_type(int socket) {
	e_query qType;
	recv(socket, &qType, sizeof(qType), 0);
	return qType;
}

e_cons_type recv_cons_type(int socket) {
	e_cons_type cType;
	recv(socket, &cType, sizeof(cType), 0);
	return cType;
}

e_request_code recv_req_code(int socket) {
	e_request_code reqCode;
	recv(socket, &reqCode, sizeof(reqCode), 0);
	return reqCode;
}

e_response_code recv_res_code(int socket) {
	e_response_code resCode;
	recv(socket, &resCode, sizeof(resCode), 0);
	return resCode;
}
