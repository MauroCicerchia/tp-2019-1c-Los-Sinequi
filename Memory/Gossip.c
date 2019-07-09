#include"Gossip.h"

void execute_gossip_client(t_config* config,t_log* logger){
	char* seed_ip = config_get_string_value(config, "IP_SEED");
	char* seed_port = config_get_string_value(config, "PUERTO_SEED");
	log_info(logger,"Intentando conectarse a la seed: %s - %s",seed_ip,seed_port);
	int seed_socket = connectToServer(seed_ip,seed_port);

	if(seed_socket == -1){
		log_warning(logger,"La seed no se ha inicializado aun");
		return;
	}
	else{
		log_info(logger,"La seed ha sido exitosamente conectada");
		send_req_code(seed_socket,REQUEST_GOSSIP);
		//send mi tabla
		send_gossip_table(seed_socket);
		e_response_code response = recv_res_code(seed_socket);
		if(response == RESPONSE_SUCCESS){
			//recibo mis tablas
			recv_gossip_table(seed_socket,config,logger);
		log_info(logger,"Tabla gossip actualizada correctamente");

		}
		else{
			log_error(logger,"Ha fallado la consulta de gossip table a seed");
		}
		close(seed_socket);
		return;
	}
}

void execute_gossip_server(int socket_gossip,t_config* config,t_log* logger){
	//llega y actualizo tabla
	recv_gossip_table(socket_gossip,config,logger);
	send_res_code(socket_gossip,RESPONSE_SUCCESS);
	send_gossip_table(socket_gossip);
	//envio mis tablas
}

void recv_gossip_table(int seed_socket,t_config* config,t_log* logger){
	int mem_count = recv_int(seed_socket);
		for(int i=0;i<mem_count;i++){
			char* ip_mem = recv_str(seed_socket);
			char* port_mem = recv_str(seed_socket);
			add_to_gossip_table(ip_mem,port_mem,config,logger);
	}
}

void send_gossip_table(int socket_gossip){
	int mem_num = list_size(gossip_table);
	send_int(socket_gossip,mem_num);
	memory* mem;
	for(int i=0;i<mem_num;i++){
		mem = list_get(gossip_table,i);
		send_str(socket_gossip,mem->memory_ip);
		send_str(socket_gossip,mem->memory_port);
	}
}

void add_to_gossip_table(char* ip_mem, char* port_mem, t_config* config, t_log* logger){
	memory* new_memory = memory_create(ip_mem,port_mem);

	bool isMemory(void* mem){
		return((strcasecmp(((memory*) mem)->memory_ip,ip_mem)==0 && strcasecmp(((memory*) mem)->memory_port,port_mem)==0));
	}

	bool is_already_in = list_any_satisfy(gossip_table, isMemory);

	if(!is_already_in){
		list_add(gossip_table,new_memory);
		log_info(logger,"Nueva memoria agregada a la tabla gossip");
	}
	else{
		log_info(logger,"No se agrego la memoria a la tabla gossip, ya estaba cargada");
	}
	return;
}

memory* memory_create(char* ip,char* port){
	memory* mem = (memory*)malloc(sizeof(memory));

	mem->memory_ip = ip;
	mem->memory_port = port;

	return mem;
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

void print_mem(void* mem){
	memory* memo = (memory*) mem;
	printf("Memoria: %s - %s\n",memo->memory_ip,memo->memory_port);
}
void print_gossip_table(){
	list_iterate(gossip_table,print_mem);
}











