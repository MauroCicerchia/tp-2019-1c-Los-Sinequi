#include"Gossip.h"



void execute_gossip_client(t_config* config,t_log* logger){
	char* seed_ip = config_get_string_value(config, "IP_SEED");
	char* seed_port = config_get_string_value(config, "PUERTO_SEED");

	int seed_socket = connectToServer(seed_ip,seed_port);
	if(seed_socket == -1){
		log_error(logger,"La seed no se ha inicializado aun");
		return;
	}
	else{
		log_info(logger,"La seed ha sido exitosamente conectada");
		send_req_code(seed_socket,REQUEST_GOSSIP);
		e_response_code response = recv_res_code(seed_socket);
		if(response == RESPONSE_SUCCESS){
			int mem_count = recv_int(seed_socket);
			for(int i=0;i<mem_count;i++){
				char* ip_mem = recv_str(seed_socket);
				char* port_mem = recv_str(seed_socket);
				add_to_gossip_table(ip_mem,port_mem,config,logger);
			}
		log_info(logger,"Tabla gossip actualizada correctamente");
		}
		else{
			log_error(logger,"Ha fallado la consulta de gossip table a seed");
		}
		return;
	}
}

void execute_gossip_server(int socket_gossip,t_config* config,t_log* logger){

	int mem_num = list_size(gossip_table);
	send_int(socket_gossip,mem_num);
	memory* mem;//CONSULTAR A MAURITE AGAIN
	for(int i=0;i<mem_num;i++){
		mem = list_get(gossip_table,i);
		send_str(socket_gossip,mem->memory_ip);  //ACA SE PODRIA PAQUETIZAR?
		send_str(socket_gossip,mem->memory_port);
	}
}

void add_to_gossip_table(char* ip_mem, char* port_mem, t_config* config, t_log* logger){
	memory* new_memory; //HAY QUE RESERVARLE MEMORIA CON MALLOC, CONSULTAR A MAURITE, AMO Y SEÑOR DE MALLOCS Y SOCKETS
	new_memory->memory_ip = ip_mem;
	new_memory->memory_port = port_mem;

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
