//#include "connection.h"
//
//void *listen_client()
//{
//	int socket = createServer(ip,port);
//	if(socket == -1) {
//		log_error(logger,"No se pudo crear el servidor");
//		exit(1);
//	}
//	while(1) {
//		int cliSocket = connectToClient(socket);
//
//		if(cliSocket == -1) {
//			printf("No se pudo conectar con el cliente\n");
//			exit(1);
//		}
//
//
//		e_request_code rc = recv_req_code(cliSocket);
//		switch(rc){
//			case REQUEST_VALUESIZE: send_int(cliSocket,valueSize); break;
//			case REQUEST_QUERY: process_query_from_client(cliSocket);
//		}
//
//		closeConnection(socket);
//	}
//}
//
//
//void process_query_from_client(int client)
//{
//	e_query opCode;
//	recv(client, &opCode, sizeof(opCode), 0);
//
//	char *table, *value;
//	int key, part, compTime, status,timeStamp;
//	char *consType;
//	char *sKey, *sTimeStamp, *sPart, *sCTime;
//	metadata *tableMetadata;
//
//	switch(opCode) {
//
//		case QUERY_SELECT:
//
//			table = recv_str(client);
//			key = recv_int(client);
//			sKey = string_itoa(key);
//			char *response = qselect(table, sKey);
//			free(sKey);
//
//			if(response != NULL) {
//				send_res_code(client, RESPONSE_SUCCESS);
//				send_str(client, response);
//			} else {
//				send_res_code(client, RESPONSE_ERROR);
//			}
//			break;
//
//
//		case QUERY_INSERT:
//
//			table = recv_str(client);
//			key = recv_int(client);
//			value = recv_str(client);
//			timeStamp = conn_getCurrentTime(); //agregar mi timestamp
//			sKey = string_itoa(key);
//			sTimeStamp = string_itoa(timeStamp);
//
//			status = qinsert(table, sKey, value,sTimeStamp);
//			free(sKey); free(sTimeStamp);
//			if(status)
//				send_res_code(client, RESPONSE_SUCCESS);
//			else
//				send_res_code(client, RESPONSE_ERROR);
//
//			break;
//
//
//		case QUERY_CREATE:
//
//			table = recv_str(client);
//			consType = recv_str(client);
//			part = recv_int(client);
//			compTime = recv_int(client);
//
//			sPart = string_itoa(part);
//			sCTime = string_itoa(compTime);
//
//			status = qcreate(table, consType, sPart, sCTime);
//			if(status)
//				send_res_code(client, RESPONSE_SUCCESS);
//
//			else
//				send_res_code(client, RESPONSE_ERROR);
//
//			break;
//
//
//		case QUERY_DESCRIBE:
//
//			table = recv_str(client);
//
//			if(!strcmp(table,"NULL")) { //todas las tablas
//
//				t_list *tables = fs_getAllTables();
//
//				for(int i = 0; i < list_size(tables); i++){
//					tableMetadata = qdescribe(list_get(tables,i));
//					if(tableMetadata != NULL){
//						send_res_code(client, RESPONSE_SUCCESS);
//						send_str(client,tableMetadata->consistency);
//						send_str(client,tableMetadata->partitions);
//						send_str(client,tableMetadata->ctime);
//
//						free(tableMetadata->consistency); free(tableMetadata->ctime); free(tableMetadata->partitions);
//						free(tableMetadata);
//					}
//					else send_res_code(client, RESPONSE_ERROR);
//				}
//			}
//			else { //una sola tabla
//				tableMetadata = qdescribe(table);
//				if(tableMetadata != NULL){
//					send_res_code(client, RESPONSE_SUCCESS);
//					send_str(client,tableMetadata->consistency);
//					send_str(client,tableMetadata->partitions);
//					send_str(client,tableMetadata->ctime);
//
//					free(tableMetadata->consistency); free(tableMetadata->ctime); free(tableMetadata->partitions);
//					free(tableMetadata);
//				}else{
//					send_res_code(client, RESPONSE_ERROR);
//				}
//			}
//			break;
//
//
//		case QUERY_DROP:
//
//			table = recv_str(client);
//			status = qdrop(table);
//			if(status)
//				send_res_code(client, RESPONSE_SUCCESS);
//			else
//				send_res_code(client, RESPONSE_ERROR);
//
//			break;
//
//
//		//casos imposibles
//		case QUERY_JOURNAL: send_res_code(client, RESPONSE_ERROR);break;
//		case QUERY_ADD: send_res_code(client, RESPONSE_ERROR);break;
//		case QUERY_RUN: send_res_code(client, RESPONSE_ERROR);break;
//		case QUERY_METRICS: send_res_code(client, RESPONSE_ERROR);break;
//		case QUERY_ERROR: send_res_code(client, RESPONSE_ERROR);break;
//	}
//
//}
//
//uint64_t conn_getCurrentTime()
//{
//	struct timeval tv;
//	gettimeofday(&tv,NULL);
//
//	return (uint64_t)((tv.tv_sec)*1000 + (tv.tv_usec)/1000);
//}
//
