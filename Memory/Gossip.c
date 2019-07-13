#include"Gossip.h"

void execute_gossip_client(t_log* logger,char* port,pthread_mutex_t MUTEX_GOSSIP){

	char** seed_ips = get_ip_seeds();
	char** seed_ports = get_port_seeds();

	int seeds_amount = 0;
	seeds_amount = sizeofArray(seed_ips);

	for(int i=0;i<seeds_amount;i++){
		char* seed_ip = seed_ips[i];
		char* seed_port = seed_ports[i];
		if (seed_port == NULL){
			seed_port = port; // ADRO dijo que siempre vamos a tener puerto, nunca llegariamos aca
		}
		log_info(logger,"[GOSSIPING]: Intentando conectarse a la seed: %s - %s",seed_ip,seed_port);
		int seed_socket = connectToServer(seed_ip,seed_port);

		if(seed_socket == -1){
			delete_mem_from_gossip_table(seed_ip,seed_port,logger);
			log_warning(logger,"[GOSSIPING]: La seed no se encuentra activa");
		}
		else{
			log_info(logger,"[GOSSIPING]: La seed ha sido exitosamente conectada");
			send_req_code(seed_socket,REQUEST_GOSSIP);
			//send mi tabla
			pthread_mutex_lock(&MUTEX_GOSSIP);
			send_gossip_table(seed_socket);
			pthread_mutex_unlock(&MUTEX_GOSSIP);
			e_response_code response = recv_res_code(seed_socket);
			if(response == RESPONSE_SUCCESS){
				//recibo mis tablas
				pthread_mutex_lock(&MUTEX_GOSSIP);
				recv_gossip_table(seed_socket,logger);
				pthread_mutex_unlock(&MUTEX_GOSSIP);
			log_info(logger,"[GOSSIPING]: Tabla gossip actualizada correctamente");

			}
			else{
				log_error(logger,"Ha fallado la consulta de gossip table a seed");
			}
			close(seed_socket);
		}
	}
	if(seeds_amount != 0) {
		string_iterate_lines(seed_ips,string_destroy_char);
		string_iterate_lines(seed_ports,string_destroy_char);
	}

	free(seed_ips);
	free(seed_ports);

}


void execute_gossip_server(int socket_gossip,t_log* logger,pthread_mutex_t MUTEX_GOSSIP){
	//llega y actualizo tabla
	pthread_mutex_lock(&MUTEX_GOSSIP);
	recv_gossip_table(socket_gossip,logger);
	pthread_mutex_unlock(&MUTEX_GOSSIP);
	send_res_code(socket_gossip,RESPONSE_SUCCESS);
	pthread_mutex_lock(&MUTEX_GOSSIP);
	send_gossip_table(socket_gossip);
	pthread_mutex_unlock(&MUTEX_GOSSIP);
	//envio mis tablas
}

void recv_gossip_table(int seed_socket,t_log* logger){
	int mem_count = recv_int(seed_socket);
	for(int i=0;i<mem_count;i++){
		int num = recv_int(seed_socket);
		char* ip_mem = recv_str(seed_socket);
		char* port_mem = recv_str(seed_socket);
		add_to_gossip_table(ip_mem,port_mem,num,logger);
		free(ip_mem);
		free(port_mem);
	}
}

void send_gossip_table(int socket_gossip){
	int mem_num = list_size(gossip_table);
	send_int(socket_gossip,mem_num);
	memory* mem;
	for(int i=0;i<mem_num;i++){
		mem = list_get(gossip_table,i);
		send_int(socket_gossip,mem->memory_number);
		send_str(socket_gossip,mem->memory_ip);
		send_str(socket_gossip,mem->memory_port);
	}
}

void add_to_gossip_table(char* ip_mem, char* port_mem,int num_mem ,t_log* logger){
	memory* new_memory = memory_create(ip_mem,port_mem,num_mem);

	bool isMemory(void* mem){
		return(((memory*)mem)->memory_number == num_mem);
	}

	bool is_already_in = list_any_satisfy(gossip_table, isMemory);

	if(!is_already_in){
		list_add(gossip_table,new_memory);
		log_info(logger,"Nueva memoria agregada a la tabla gossip");
	}
	else{
		log_info(logger,"No se agrego la memoria a la tabla gossip, ya estaba cargada");
		memory_destroy(new_memory);
	}
	return;
}

memory* memory_create(char* ip,char* port,int number){
	memory* mem = (memory*)malloc(sizeof(memory));

	mem->memory_ip = string_duplicate(ip);
	mem->memory_port = string_duplicate(port);
	mem->memory_number = number;

	return mem;
}

void delete_mem_from_gossip_table(char*ip, char*port,t_log* logger){
	bool deleteMem(void* mem){
		memory* memo = (memory*) mem;
		if(strcmp(memo->memory_ip,ip)==0 && strcmp(memo->memory_port,port)==0){
			log_info(logger,"[GOSSIPING]: Se desconecto la seed %s - %s, se elimina de la tabla de gossip",ip,port);
			return true;
		}else{
			return false;
		}
	}
	list_remove_and_destroy_by_condition(gossip_table,deleteMem,memory_destroy);
}

void memory_destroy(void* mem){
	memory* memo = (memory*) mem;
	free(memo->memory_ip);
	free(memo->memory_port);
	free(memo);
}

void gossip_table_destroy(){
	list_destroy_and_destroy_elements(gossip_table,memory_destroy);
}







