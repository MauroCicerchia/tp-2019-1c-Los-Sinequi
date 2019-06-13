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
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar describe (No existe la tabla).");
		return;
	}
	t_memory *mem = get_memory_for_table(t);

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);
	send_req_code(memSocket, REQUEST_QUERY);
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
	char *table = args[1];
	int key = strtol(args[2], NULL, 10);
	char *value = args[3];

//	Paquetizar
	t_package *p = create_package(queryType);
	add_to_package(p, (void*)table, sizeof(char) * (strlen(table) + 1));
	add_to_package(p, (void*)&key, sizeof(key));
	add_to_package(p, (void*)value, sizeof(char) * (strlen(value) + 1));

//	obtener memoria segun criterio
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar describe (No existe la tabla).");
		return;
	}
	t_memory *mem = get_memory_for_table(t);

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {

	} else {
		log_error(logger, " >> Error al realizar insert en memoria.");
	}
	close(memSocket);
	return;
}

void qCreate(char** args, t_log *logger) {
//	Convertir args en tipo valido
	e_query queryType = getQueryType(args[0]);
	char *table = args[1];
	char *consType = args[2];
	int part = strtol(args[3], NULL, 10);
	int compTime = strtol(args[4], NULL, 10);

//	Paquetizar
	t_package *p = create_package(queryType);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);
	add_to_package(p, (void*)consType, sizeof(char) * strlen(consType) + 1);
	add_to_package(p, (void*)&part, sizeof(part));
	add_to_package(p, (void*)&compTime, sizeof(compTime));

//	obtener memoria segun criterio
	t_memory *mem = get_memory_of_cons_type(CONS_SC);

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		add_table(table_create(table, consType, part, compTime));
	} else {
		log_error(logger, " >> Error al realizar create en memoria.");
	}
	close(memSocket);
	return;
}

void qDescribe(char** args, t_log *logger) {
//	Convertir args en tipo valido
	e_query queryType = getQueryType(args[0]);
	char *table = args[1];

//	obtener memoria segun criterio
	t_memory *mem = get_memory_of_cons_type(CONS_SC);

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	if(table != NULL) {
//		Paquetizar
		t_package *p = create_package(queryType);
		add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);

		send_req_code(memSocket, REQUEST_QUERY);
		send_package(p, memSocket);

		e_response_code r = recv_res_code(memSocket);

		if(r == RESPONSE_SUCCESS) {
			e_cons_type consType = recv_cons_type(memSocket);
			int part = recv_int(memSocket);
			int compTime = recv_int(memSocket);
			update_table(table, consType, part, compTime);
			log_info(logger, " >> Metadata de tabla actualizada.");
		} else {
			log_error(logger, " >> Error al realizar describe en memoria.");
		}
	} else {
		send_req_code(memSocket, REQUEST_QUERY);
		send_q_type(memSocket, QUERY_DESCRIBE);
		send_int(memSocket, 0);

		e_response_code r = recv_res_code(memSocket);

		if(r == RESPONSE_SUCCESS) {
			int q = recv_int(memSocket);
			for(int i = 0; i < q; i++) {
				char *name = recv_str(memSocket);
				e_cons_type consType = recv_cons_type(memSocket);
				int part = recv_int(memSocket);
				int compTime = recv_int(memSocket);
				update_table(name, consType, part, compTime);
			}
			log_info(logger, " >> Metadata de todas las tablas actualizada.");
		} else {
			log_error(logger, " >> Error al realizar describe en memoria.");
		}
	}
	close(memSocket);
	return;
}

void qDrop(char** args, t_log *logger) {
//	Convertir args en tipo valido
	e_query queryType = getQueryType(args[0]);
	char *table = args[1];

//	Paquetizar
	t_package *p = create_package(queryType);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);

//	obtener memoria segun criterio
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar describe (No existe la tabla).");
		return;
	}
	t_memory *mem = get_memory_for_table(t);

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		drop_table(table);
	} else {
		log_error(logger, " >> Error al realizar drop en memoria.");
	}
	close(memSocket);
	return;
}
