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

char* send_select_to_FS(char* segmentID, uint16_t key, t_config* config,t_log* logger){

	printf("%s, %d", segmentID, (int)key);

	t_package *p = create_package(QUERY_SELECT);
	add_to_package(p, (void*)segmentID, sizeof(char) * (strlen(segmentID) + 1));
	add_to_package(p, (void*)&key, sizeof(key));


	int FS_socket = connect_to_FS(config,logger);

	send_req_code(FS_socket, REQUEST_QUERY);
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

int request_valuesize_to_FS(t_config* config, t_log* logger){

	int FS_socket = connect_to_FS(config,logger);
	send_req_code(FS_socket,REQUEST_VALUESIZE);

	e_response_code r = recv_res_code(FS_socket);

	if(r == RESPONSE_SUCCESS) {
			int valuesize = recv_int(FS_socket);
			log_info(logger,"Retorno valuesize de FS");
			return valuesize;
		} else {
			log_error(logger, "Error al consultar valuesize a FS");
			}
		close(FS_socket);
	return 0;
}

int send_create_to_FS(char* table,char* consType, char *part, char *compTime ,t_config* config, t_log* logger){

	//	Paquetizar
	t_package *p = create_package(QUERY_CREATE);
	add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);
	add_to_package(p, (void*)consType, sizeof(char) * strlen(consType) + 1);
	add_to_package(p, (void*)part, sizeof(char)* strlen(part) + 1);
	add_to_package(p, (void*)compTime, sizeof(char) * strlen(compTime) + 1);

	int FS_socket = connect_to_FS(config,logger);

	send_req_code(FS_socket, REQUEST_QUERY);
	send_package(p, FS_socket);

	e_response_code r = recv_res_code(FS_socket);

	printf("responde fs");
	if(r == RESPONSE_SUCCESS) {
		log_info(logger,"Se creo la tabla en FS");
		close(FS_socket);
		return 0;
	} else {
		log_error(logger, " Error al realizar create en FS");
		close(FS_socket);
		return -1;
	}

}

t_list* send_describe_to_FS(char*table,t_config* config,t_log* logger){
	t_list* metadata_list = list_create();
	int FS_socket = connect_to_FS(config,logger);

	if(table != NULL) {
		//Paquetizar
		t_package *p = create_package(QUERY_DESCRIBE);
		add_to_package(p, (void*)table, sizeof(char) * strlen(table) + 1);

		send_req_code(FS_socket, REQUEST_QUERY);
		send_package(p, FS_socket);

		e_response_code r = recv_res_code(FS_socket);

		if(r == RESPONSE_SUCCESS) {
			//aca recibimos la metadata de una tabla

			metadata* aMetaData = (metadata*)malloc(sizeof(metadata));
			aMetaData->tableName = recv_str(FS_socket);
			aMetaData->consType = recv_str(FS_socket);
			aMetaData->partNum = recv_str(FS_socket);
			aMetaData->compTime = recv_str(FS_socket);
      //	printf("%s:\n	%s, %s, %s\n",tableName,consistencyType,partitions,compactTime);
			list_add(metadata_list,aMetaData);
		}
		else {

			log_error(logger, "Error al realizar describe en FS");
		}
	} else {
		send_req_code(FS_socket, REQUEST_QUERY);
		send_q_type(FS_socket, QUERY_DESCRIBE);
		send_str(FS_socket, "");

		e_response_code r = recv_res_code(FS_socket);

		if(r == RESPONSE_SUCCESS) {

			//aca recibimos la metadata de todas las tablas existentes
			int cant_tablas = recv_int(FS_socket);
			for(int i=0;i<cant_tablas;i++){
				metadata* aMetaData = (metadata*)malloc(sizeof(metadata));
				aMetaData->tableName = recv_str(FS_socket);
				aMetaData->consType = recv_str(FS_socket);
				aMetaData->partNum = recv_str(FS_socket);
				aMetaData->compTime = recv_str(FS_socket);
          //	printf("%s:\n	%s, %s, %s\n",tableName,consistencyType,partitions,compactTime);
				list_add(metadata_list,aMetaData);
			}
		}
		else {

			log_error(logger, "Error al realizar describe en FS.");
		}
	}
	close(FS_socket);
	return(metadata_list);
}

void send_insert_to_FS(char* table,uint16_t key,char* value,t_config* config,t_log* logger){

	//	Paquetizar
	t_package *p = create_package(QUERY_INSERT);
	add_to_package(p, (void*)table, sizeof(char) * (strlen(table) + 1));
	add_to_package(p, (void*)&key, sizeof(key));
	add_to_package(p, (void*)value, sizeof(char) * (strlen(value) + 1));

	int FS_socket = connect_to_FS(config,logger);

	send_req_code(FS_socket, REQUEST_QUERY);
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

	int FS_socket = connect_to_FS(config,logger);

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




int connect_to_FS(t_config* config, t_log* logger){
	char* FS_ip = config_get_string_value(config, "IP_FS");
	char* FS_port = config_get_string_value(config, "PUERTO_FS");
	log_info(logger,FS_ip);
	log_info(logger,FS_port);
	int socket = connectToServer(FS_ip,FS_port);
	if(socket == -1){
		log_error(logger,"La conexion con FS fallo");
		exit (-1);
	}
	else{
		log_info(logger,"La conexion con FS fue exitosa");
		return socket;
	}
}

void metadata_destroy(void* aMD){
	metadata *md = (metadata*)aMD;
	free(md->compTime);
	free(md->consType);
	free(md->partNum);
	free(md->tableName);
	free(md);
}
