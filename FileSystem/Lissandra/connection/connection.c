#include "connection.h"

void *listen_client()
{
	log_info(logger,"[RX/TX]: Iniciando servidor...");
	int socket = createServer(ip,port);
	if(socket == -1) {
		log_error(logger,"[RX/TX]: No se pudo crear el servidor");
		exit(1);
	}
	log_info(logger,"[RX/TX]: Servidor creado");
	while(1) {
		int cliSocket = connectToClient(socket);

		if(cliSocket == -1) {
			log_warning(logger,"[RX/TX]: No se pudo conectar con el cliente\n");
		}
		else{
//			sem_wait(&MAX_CLIENTS);
			pthread_t queryThread;

			pthread_create(&queryThread, NULL, attendClient, (void*)cliSocket);
			pthread_detach(queryThread);
		}

//		close(cliSocket);
	}
}

void *attendClient(void *socket){
	int cliSocket = (int)socket;
	log_info(logger,"[RX/TX]: Recibo request de cliente");
			e_request_code rc = recv_req_code(cliSocket);

			switch(rc){
				case REQUEST_VALUESIZE: send_int(cliSocket,valueSize); break;
				case REQUEST_QUERY: process_query_from_client(cliSocket); break;

				//imposible
				case REQUEST_GOSSIP: break;
				case REQUEST_JOURNAL: break;
			}
			log_info(logger,"[RX/TX]: Request enviada");
//			sem_post(&MAX_CLIENTS);
			return NULL;
}


void process_query_from_client(int client)
{
	e_query opCode;
	recv(client, &opCode, sizeof(opCode), 0);

	char *table, *value;
	int status;
	uint16_t key;
	uint64_t timestamp;
	char *consType;
	char *sKey, *sTimeStamp, *sPart, *sCTime;
	metadata *tableMetadata;

	switch(opCode) {

		case QUERY_SELECT:

			table = recv_str(client);
			key = recv_int(client);
			sKey = string_itoa((int)key);
			char *response = qselect(table, sKey);
			free(sKey);
			free(table);

			if(response != NULL) {
				send_res_code(client, RESPONSE_SUCCESS);
				send_str(client, response);
			} else {
				send_res_code(client, RESPONSE_ERROR);
			}
			free(response);
			break;


		case QUERY_INSERT:

			table = recv_str(client);
			key = recv_int(client);
			timestamp = recv_timestamp(client);
			value = recv_str(client);
			sKey = string_itoa((int)key);

			sTimeStamp = string_from_format("%llu", timestamp);
//			string_from_format("%llu",conn_getCurrentTime());

			status = qinsert(table, sKey, value,sTimeStamp);
			free(sKey); free(sTimeStamp);
			free(table); free(value);

			if(status)
				send_res_code(client, RESPONSE_SUCCESS);
			else
				send_res_code(client, RESPONSE_ERROR);

			break;


		case QUERY_CREATE:

			table = recv_str(client);
			consType = recv_str(client);
			sPart = recv_str(client);
			sCTime = recv_str(client);
			status = qcreate(table, consType, sPart, sCTime);
			free(table); free(consType); free(sPart); free(sCTime);
			if(status)
				send_res_code(client, RESPONSE_SUCCESS);

			else
				send_res_code(client, RESPONSE_ERROR);

			break;


		case QUERY_DESCRIBE:

			table = recv_str(client);

			if(!strcmp(table,"")) { //todas las tablas

				t_list *tables = fs_getAllTables();
				send_res_code(client, RESPONSE_SUCCESS);
				send_int(client,list_size(tables));
				for(int i = 0; i < list_size(tables); i++){
					tableMetadata = qdescribe(list_get(tables,i));
					if(tableMetadata != NULL){
						send_str(client,list_get(tables,i));
						send_str(client,tableMetadata->consistency);
						send_str(client,tableMetadata->partitions);
						send_str(client,tableMetadata->ctime);

						free(tableMetadata->consistency); free(tableMetadata->ctime); free(tableMetadata->partitions);
						free(tableMetadata);
					}
					else send_res_code(client, RESPONSE_ERROR);
				}
				free(table);
				list_destroy_and_destroy_elements(tables, free);
			}
			else { //una sola tabla
				tableMetadata = qdescribe(table);
				if(tableMetadata != NULL){
					send_res_code(client, RESPONSE_SUCCESS);
					send_str(client,table);
					send_str(client,tableMetadata->consistency);
					send_str(client,tableMetadata->partitions);
					send_str(client,tableMetadata->ctime);

					free(tableMetadata->consistency); free(tableMetadata->ctime); free(tableMetadata->partitions);
					free(tableMetadata);
				}else{
					send_res_code(client, RESPONSE_ERROR);
				}
				free(table);
			}
			break;


		case QUERY_DROP:

			table = recv_str(client);
//			printf("%s",table);
			status = qdrop(table);
			free(table);
			if(status)
				send_res_code(client, RESPONSE_SUCCESS);
			else
				send_res_code(client, RESPONSE_ERROR);

			break;


		//casos imposibles
		case QUERY_JOURNAL: send_res_code(client, RESPONSE_ERROR);break;
		case QUERY_ADD: send_res_code(client, RESPONSE_ERROR);break;
		case QUERY_RUN: send_res_code(client, RESPONSE_ERROR);break;
		case QUERY_METRICS: send_res_code(client, RESPONSE_ERROR);break;
		case QUERY_ERROR: send_res_code(client, RESPONSE_ERROR);break;
	}

}

uint64_t conn_getCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	uint64_t  x = (uint64_t)( (tv.tv_sec)*1000 + (tv.tv_usec)/1000 );
	return x;
}

