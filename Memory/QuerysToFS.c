#include"QuerysToFS.h"


//void send_frame_to_FS(int frame_num, int socket){
//
//	t_package* p = create_package(QUERY_INSERT);
//
//}

/*
 * INSERT -> FS responde succes/error
 * SELECT -> FS responde (succes + resultado)/error
 * CREATE -> FS responde succes/error
 * DESCRIBE-> FS responde (succes + cantidad de tablas )/error
 * 			while(cant_tablas)(succes + send_table() )
 *
 * DROP -> FS responde succes/error
 *
 */



char* send_select_to_FS(char* segmentID, int key, t_config* config,t_log* logger){

	t_package *p = create_package(QUERY_SELECT);
	add_to_package(p, (void*)segmentID, sizeof(char) * (strlen(segmentID) + 1));
	add_to_package(p, (void*)&key, sizeof(key));



	int FS_socket = connect_to_FS(config);

	//send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, FS_socket);

	e_response_code r = recv_res_code(FS_socket);

	if(r == RESPONSE_SUCCESS) {
		char *value = recv_str(FS_socket);
		log_info(logger,"Retorno algo de FS");
		return value;
	} else {
		log_error(logger, "Error al realizar select en FS");
		}
	close(FS_socket);
	return NULL;
}

void send_create_to_FS(char* table,char* consType, int part, int compTime,t_config* config, t_log* logger){

	//	Paquetizar
	t_package *p = create_package(QUERY_CREATE);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);
	add_to_package(p, (void*)consType, sizeof(char) * strlen(consType) + 1);
	add_to_package(p, (void*)&part, sizeof(part));
	add_to_package(p, (void*)&compTime, sizeof(compTime));

	int FS_socket = connect_to_FS(config);

	//send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, FS_socket);

	e_response_code r = recv_res_code(FS_socket);

	if(r == RESPONSE_SUCCESS) {
		log_info(logger,"Se creo la tabla en FS");
	} else {
		log_error(logger, " Error al realizar create en FS");
	}
	close(FS_socket);
	return;
}

//no debe ser void, debemos recibir metadata de tablas
void send_describe_to_FS(char*table,t_config* config,t_log* logger){

	int FS_socket = connect_to_FS(config);

	if(table != NULL) {
	//	Paquetizar
		t_package *p = create_package(QUERY_DESCRIBE);
		add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);

		//send_req_code(FS_socket, REQUEST_QUERY);
		send_package(p, FS_socket);

		e_response_code r = recv_res_code(FS_socket);

		if(r == RESPONSE_SUCCESS) {
			//aca recibimos la metadata de una tabla
		} else {
			log_error(logger, "Error al realizar describe en FS");
		}
	} else {
		//send_req_code(memSocket, REQUEST_QUERY);
		send_q_type(FS_socket, QUERY_DESCRIBE);
		send_int(FS_socket, 0);

		e_response_code r = recv_res_code(FS_socket);

		if(r == RESPONSE_SUCCESS) {
			//aca recibimos la metadata de todas las tablas existentes
		} else {
			log_error(logger, "Error al realizar describe en FS.");
		}
	}
	close(FS_socket);
	return;
}

void send_insert_to_FS(char* table,int key,char* value,t_config* config,t_log* logger){

	//	Paquetizar
	t_package *p = create_package(QUERY_INSERT);
	add_to_package(p, (void*)table, sizeof(char) * (strlen(table) + 1));
	add_to_package(p, (void*)&key, sizeof(key));
	add_to_package(p, (void*)value, sizeof(char) * (strlen(value) + 1));

	int FS_socket = connect_to_FS(config);

//	send_req_code(memSocket, REQUEST_QUERY);
	send_package(p, FS_socket);

	e_response_code r = recv_res_code(FS_socket);

	if(r == RESPONSE_SUCCESS) {
		log_info(logger,"Se realizo el insert correctamente en FS");
	} else {
		log_error(logger, "Error al realizar insert en FS");
	}
	close(FS_socket);
	return;
}

void send_drop_to_FS(char* table,t_config* config,t_log* logger){

	//	Paquetizar
	t_package *p = create_package(QUERY_DROP);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);

	int FS_socket = connect_to_FS(config);

	send_req_code(FS_socket, REQUEST_QUERY);
	send_package(p, FS_socket);

	e_response_code r = recv_res_code(FS_socket);

	if(r == RESPONSE_SUCCESS) {
		log_info(logger,"Se realizo drop correctamente en FS");
	} else {
		log_error(logger, "Error al realizar drop en FS");
	}
	close(FS_socket);
	return;
}




int connect_to_FS(t_config* config){
	char* FS_ip = config_get_string_value(config, "IP_FS");
	int FS_port = config_get_int_value(config, "PUERTO_FS");
	return connectToServer(FS_ip,FS_port);
}
