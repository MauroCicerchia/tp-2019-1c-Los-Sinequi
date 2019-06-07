#include"QueryExec.h"

void qSelect(char** args, t_log *logger) {

//	Convertir args en tipo valido
	e_query queryType = getQueryType(args[0]);
	char *table = args[1];
	int key = strtol(args[2], NULL, 10);

//	Paquetizar
	t_package *p = create_package(queryType);
	add_to_package(p, (void*)table, sizeof(char) * (strlen(table) + 1));
	add_to_package(p, (void*)&key, sizeof(key));

//	obtener memoria segun criterio
	t_memory *mem = get_memory_of_cons_type(CONS_SC);

	//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);
	send_package(p, memSocket);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		char *value = recv_str(memSocket);

		FILE * output = txt_open_for_append("../output.txt");
		txt_write_in_file(output, "SELECT ");
		string_trim(&args[1]);
		string_trim(&args[2]);
		string_trim(&value);
		txt_write_in_file(output, args[1]);
		txt_write_in_file(output, " ");
		txt_write_in_file(output, args[2]);
		txt_write_in_file(output, " ::\n	");
		txt_write_in_file(output, value);
		txt_write_in_file(output, "\n");

	} else {
//		notificar error
		log_error(logger, " >> Error al realizar select en memoria.");
	}
	close(memSocket);
	return;
}

void qInsert(char** args, t_log *logger) {
//	Convertir args en tipo valido
	e_query queryType = getQueryType(args[0]);
	char *tabla = args[1];
	int key = strtol(args[2], NULL, 10);
	char *value = args[3];

//	Paquetizar
	t_package *p = create_package(queryType);
	add_to_package(p, (void*)tabla, sizeof(char) * (strlen(tabla) + 1));
	add_to_package(p, (void*)&key, sizeof(key));
	add_to_package(p, (void*)value, sizeof(char) * (strlen(value) + 1));

//	obtener memoria segun criterio
	t_memory *mem = get_memory_of_cons_type(CONS_SC);

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);
	send_package(p, memSocket);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {

	} else {
//		notificar error
		log_error(logger, " >> Error al realizar insert en memoria.");
	}
	close(memSocket);
	return;
}

void qCreate(char** args, t_log *logger) {
//	Convertir args en tipo valido
//	Paquetizar
//	Enviar query a memoria
//	Confirmar creacion de tabla
//	Crear tabla en kernel
}

void qDescribe(char** args, t_log *logger) {
//	Convertir args en tipo valido
//	Paquetizar
//	Enviar query a memoria
//	Actualizar estructuras administrativas
}

void qDrop(char** args, t_log *logger) {
//	Convertir args en tipo valido
//	Paquetizar
//	Enviar query a memoria
//	Actualizar estructuras administrativas
}
