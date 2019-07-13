#include"QueryExec.h"

int qSelect(char *tableName, uint16_t key, t_log *logger) {
	char *table = string_duplicate(tableName);
//	Paquetizar
	t_package *p = create_package(QUERY_SELECT);
	add_to_package(p, (void*)table, sizeof(char) * (strlen(table) + 1));
	add_to_package(p, (void*)&key, sizeof(key));

//	obtener memoria segun criterio
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar select (No existe la tabla).");
		delete_package(p);
		free(table);
		return -1;
	}
	t_memory *mem = get_memory_for_query(t, key);

	if(mem == NULL) {
		log_error(logger, "No se pudo realizar la query. No hay memorias disponibles para esta consistencia.");
		delete_package(p);
		free(table);
		return -1;
	}

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	if(memSocket == -1) {
		log_warning(logger, "La memoria a la que se envio la query no responde. Eliminando memoria e intentando de nuevo.");
		remove_memory(mem->mid);
		if(memory_is_shc(mem)) {
			update_shc();
		}
		if(memory_is_sc(mem)) {
			remove_sc(mem->mid);
			update_sc();
		}
		int status = qSelect(table, key, logger);
		free(table);
		memory_destroy(mem);
		delete_package(p);
		return status;
	}

	log_info(logger, " >> Enviando select a memoria %d.", mem->mid);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);
	delete_package(p);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		char *value = recv_str(memSocket);
		output_select(table, key, value);
		free(value);
	} else {
		log_error(logger, " >> Error al realizar select en memoria.");
		close(memSocket);
		free(table);
		return 0;
	}
	close(memSocket);

	mem->totalOperations++;

	free(table);

	return 1;
}

int qInsert(char* table, uint16_t key, char *value, t_log *logger) {
//	Paquetizar
	t_package *p = create_package(QUERY_INSERT);
	add_to_package(p, (void*)table, sizeof(char) * (strlen(table) + 1));
	add_to_package(p, (void*)&key, sizeof(key));
	add_to_package(p, (void*)value, sizeof(char) * (strlen(value) + 1));

//	obtener memoria segun criterio
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar insert (No existe la tabla).");
		delete_package(p);
		return -1;
	}

	t_memory *mem = get_memory_for_query(t, key);

	if(mem == NULL) {
		log_error(logger, "No se pudo realizar la query. No hay memorias disponibles para esta consistencia.");
		delete_package(p);
		return -1;
	}

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	if(memSocket == -1) {
		log_warning(logger, "La memoria a la que se envio la query no responde. Eliminando memoria e intentando de nuevo.");
		remove_memory(mem->mid);
		if(memory_is_shc(mem)) {
			update_shc();
		}
		if(memory_is_sc(mem)) {
			remove_sc(mem->mid);
			update_sc();
		}
		int status = qInsert(table, key, value, logger);
		memory_destroy(mem);
		delete_package(p);
		return status;
	}

	log_info(logger, " >> Enviando insert a memoria %d.", mem->mid);

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);
	delete_package(p);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		log_info(logger, " >> Insert Realizado correctamente en memoria.");
	} else {
		log_error(logger, " >> Error al realizar insert en memoria.");
		close(memSocket);
		return 0;
	}
	close(memSocket);

	mem->totalOperations++;

	return 1;
}

int qCreate(char *table, char *consType, char *part, char *compTime, t_log *logger) {
//	Paquetizar
	t_package *p = create_package(QUERY_CREATE);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);
	add_to_package(p, (void*)consType, sizeof(char) * strlen(consType) + 1);
	add_to_package(p, (void*)part, sizeof(char) * strlen(part) + 1);
	add_to_package(p, (void*)compTime, sizeof(char) * strlen(compTime) + 1);

//	obtener memoria segun criterio
	t_memory *mem = get_any_memory();

	if(mem == NULL) {
		log_error(logger, "No se pudo realizar la query. No hay memorias disponibles.");
		delete_package(p);
		return -1;
	}

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	if(memSocket == -1) {
		log_warning(logger, "La memoria a la que se envio la query no responde. Eliminando memoria e intentando de nuevo.");
		remove_memory(mem->mid);
		if(memory_is_shc(mem)) {
			update_shc();
		}
		if(memory_is_sc(mem)) {
			remove_sc(mem->mid);
			update_sc();
		}
		int status = qCreate(table, consType, part, compTime, logger);
		memory_destroy(mem);
		delete_package(p);
		return status;
	}

	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, memSocket);
	delete_package(p);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		add_table(table_create(table, getConsistencyType(consType), strtol(part, NULL, 10), strtol(compTime, NULL, 10)));
		log_info(logger, " >> Tabla nueva agregada.");
	} else {
		log_error(logger, " >> Error al realizar create en memoria.");
		close(memSocket);
		return 1;
	}
	close(memSocket);
	return 1;
}

int qDescribe(char* table, t_log *logger) {
//	obtener memoria segun criterio
	t_memory *mem = get_any_memory();

	if(mem == NULL) {
		log_error(logger, "No se pudo realizar la query. No hay memorias disponibles.");
		return -1;
	}

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	if(memSocket == -1) {
		log_warning(logger, "La memoria a la que se envio la query no responde. Eliminando memoria e intentando de nuevo.");
		remove_memory(mem->mid);
		if(memory_is_shc(mem)) {
			update_shc();
		}
		if(memory_is_sc(mem)) {
			remove_sc(mem->mid);
			update_sc();
		}
		int status = qDescribe(table, logger);
		memory_destroy(mem);
		return status;
	}

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
			free(tableName);
			log_info(logger, " >> Metadata de tabla actualizada.");
		} else {
			log_error(logger, " >> Error al realizar describe en memoria.");
			close(memSocket);
			return 0;
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
			close(memSocket);
			return 0;
		}
	}
	close(memSocket);
	return 1;
}

int qDrop(char *table, t_log *logger) {
//	Paquetizar
	t_package *p = create_package(QUERY_DROP);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);

//	obtener memoria segun criterio
	t_table *t = get_table(table);
	if(t == NULL) {
		log_error(logger, " >> Error al realizar drop (No existe la tabla).");
		delete_package(p);
		return -1;
	}

	t_memory *mem = get_any_memory();

	if(mem == NULL) {
		log_error(logger, "No se pudo realizar la query. No hay memorias disponibles.");
		delete_package(p);
		return -1;
	}

//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);

	if(memSocket == -1) {
		log_warning(logger, "La memoria a la que se envio la query no responde. Eliminando memoria e intentando de nuevo.");
		remove_memory(mem->mid);
		if(memory_is_shc(mem)) {
			update_shc();
		}
		if(memory_is_sc(mem)) {
			remove_sc(mem->mid);
			update_sc();
		}
		int status = qDrop(table, logger);
		delete_package(p);
		memory_destroy(mem);
		return status;
	}

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
	return 1;
}

int qJournal(t_memory *mem, t_log *logger) {
	int memSocket = connect_to_memory(mem->ip, mem->port);

	if(memSocket == -1) {
		log_warning(logger, "La memoria a la que se envio el journal no responde. Eliminando memoria.");
		remove_memory(mem->mid);
		if(memory_is_shc(mem)) {
			update_shc();
		}
		if(memory_is_sc(mem)) {
			remove_sc(mem->mid);
			update_sc();
		}
		memory_destroy(mem);
		return 0;
	}

	send_req_code(memSocket, REQUEST_JOURNAL);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		log_info(logger, " >> Journal enviado a memoria %d.", mem->mid);
	} else {
		log_info(logger, " >> Error al realizar journal en memoria %d.", mem->mid);
	}
	close(memSocket);
	return 1;
}

void output_select(char *table, uint16_t key, char* value) {
	char *sKey = string_itoa(key);
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
