#include"QueryExec.h"

void qSelect(char *table, uint16_t key, t_log *logger) {
//	Paquetizar
	t_package *p = create_package(QUERY_SELECT);
	add_to_package(p, (void*)table, sizeof(char) * (strlen(table) + 1));
	add_to_package(p, (void*)&key, sizeof(key));

//	obtener memoria segun criterio
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar select (No existe la tabla).");
		return;
	}
	t_memory *mem = get_memory_for_query(t, key);

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	log_info(logger, " >> Enviando select a memoria %d.", mem->mid);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);
	delete_package(p);

	log_info(logger, "algo");

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		char *value = recv_str(memSocket);
		log_info(logger, "algo %s", value);
		output_select(table, key, value);
	} else {
//		notificar error
		log_error(logger, " >> Error al realizar select en memoria.");
	}
	close(memSocket);

	mem->totalOperations++;

	return;
}

void qInsert(char* table, uint16_t key, char *value, t_log *logger) {
//	Paquetizar
	t_package *p = create_package(QUERY_INSERT);
	add_to_package(p, (void*)table, sizeof(char) * (strlen(table) + 1));
	add_to_package(p, (void*)&key, sizeof(key));
	add_to_package(p, (void*)value, sizeof(char) * (strlen(value) + 1));

//	obtener memoria segun criterio
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar insert (No existe la tabla).");
		return;
	}
	t_memory *mem = get_memory_for_query(t, key);

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	log_info(logger, " >> Enviando insert a memoria %d.", mem->mid);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);
	delete_package(p);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		log_info(logger, " >> Insert Realizado correctamente en memoria.");
	} else {
		log_error(logger, " >> Error al realizar insert en memoria.");
	}
	close(memSocket);

	mem->totalOperations++;

	return;
}

void qCreate(char *table, char *consType, char *part, char *compTime, t_log *logger) {
//	Paquetizar
	t_package *p = create_package(QUERY_CREATE);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);
	add_to_package(p, (void*)consType, sizeof(char) * strlen(consType) + 1);
	add_to_package(p, (void*)part, sizeof(char) * strlen(part) + 1);
	add_to_package(p, (void*)compTime, sizeof(char) * strlen(compTime) + 1);

//	obtener memoria segun criterio
	t_memory *mem = get_any_memory();

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);
	delete_package(p);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		add_table(table_create(table, getConsistencyType(consType), strtol(part, NULL, 10), strtol(compTime, NULL, 10)));
		log_info(logger, " >> Tabla nueva agregada.");
	} else {
		log_error(logger, " >> Error al realizar create en memoria.");
	}
	close(memSocket);
	return;
}

void qDescribe(char* table, t_log *logger) {
//	obtener memoria segun criterio
	t_memory *mem = get_any_memory();

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	if(table != NULL) {
//		Paquetizar
		t_package *p = create_package(QUERY_DESCRIBE);
		add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);

		send_req_code(memSocket, REQUEST_QUERY);
		send_package(p, memSocket);
		delete_package(p);

		e_response_code r = recv_res_code(memSocket);

		if(r == RESPONSE_SUCCESS) {
			char *tableName = recv_str(memSocket);
			char *sCType = recv_str(memSocket);
			e_cons_type consType = getConsistencyType(sCType);
			free(sCType);
			char *sPart = recv_str(memSocket);
			int part = strtol(sPart, NULL, 10);
			free(sPart);
			char *sCTime = recv_str(memSocket);
			int compTime = strtol(sCTime, NULL, 10);
			free(sCTime);
			update_table(tableName, consType, part, compTime);
			output_describe(tableName, consType, part, compTime);
			log_info(logger, " >> Metadata de tabla actualizada.");
		} else {
			log_error(logger, " >> Error al realizar describe en memoria.");
		}
	} else {
		send_req_code(memSocket, REQUEST_QUERY);
		send_q_type(memSocket, QUERY_DESCRIBE);
		send_str(memSocket, "");

		e_response_code r = recv_res_code(memSocket);

		if(r == RESPONSE_SUCCESS) {
			int q = recv_int(memSocket);
			for(int i = 0; i < q; i++) {
				char *tableName = recv_str(memSocket);
				char *sCType = recv_str(memSocket);
				e_cons_type consType = getConsistencyType(sCType);
				free(sCType);
				char *sPart = recv_str(memSocket);
				int part = strtol(sPart, NULL, 10);
				free(sPart);
				char *sCTime = recv_str(memSocket);
				int compTime = strtol(sCTime, NULL, 10);
				free(sCTime);
				update_table(tableName, consType, part, compTime);
				output_describe(tableName, consType, part, compTime);
				free(tableName);
			}
			log_info(logger, " >> Metadata de todas las tablas actualizada.");
		} else {
			log_error(logger, " >> Error al realizar describe en memoria.");
		}
	}
	close(memSocket);
	return;
}

void qDrop(char *table, t_log *logger) {
//	Paquetizar
	t_package *p = create_package(QUERY_DROP);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);

//	obtener memoria segun criterio
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar drop (No existe la tabla).");
		return;
	}

	t_memory *mem = get_any_memory();

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);
	delete_package(p);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		drop_table(table);
	} else {
		log_error(logger, " >> Error al realizar drop en memoria.");
	}
	close(memSocket);
	return;
}

void qJournal(t_memory *mem, t_log *logger) {
	int memSocket = connect_to_memory(mem->ip, mem->port);

	send_req_code(memSocket, REQUEST_JOURNAL);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		log_info(logger, " >> Journal enviado a memoria %d.", mem->mid);
	} else {
		log_info(logger, " >> Error al realizar journal en memoria %d.", mem->mid);
	}
	close(memSocket);
	return;
}

void output_select(char *table, uint16_t key, char* value) {
	char *sKey = string_itoa(key);
	string_trim(&table);
	string_trim(&sKey);
	string_trim(&value);
	FILE* output = txt_open_for_append("../select_output.txt");
	txt_write_in_file(output, "SELECT ");
	txt_write_in_file(output, table);
	txt_write_in_file(output, " ");
	txt_write_in_file(output, sKey);
	txt_write_in_file(output, " ::\n	");
	txt_write_in_file(output, value);
	txt_write_in_file(output, "\n");
	txt_close_file(output);
	free(sKey);
}

void output_describe(char *name, e_cons_type cType, int part, int compTime) {
	char buffer[20];
	FILE * output = txt_open_for_append("../describe_output.txt");
	txt_write_in_file(output, "DESCRIBE ");
	txt_write_in_file(output, name);
	txt_write_in_file(output, " ::\n	");
	txt_write_in_file(output, "C: ");
	txt_write_in_file(output, getConsistencyStr(cType));
	txt_write_in_file(output, " - ");
	txt_write_in_file(output, "P: ");
	sprintf(buffer, "%d", part);
	txt_write_in_file(output, buffer);
	txt_write_in_file(output, " - ");
	txt_write_in_file(output, "CT: ");
	sprintf(buffer, "%d", compTime);
	txt_write_in_file(output, buffer);
	txt_write_in_file(output, "\n\n");
	txt_close_file(output);
}
