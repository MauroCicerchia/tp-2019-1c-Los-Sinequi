#include"QueryExec.h"

void qSelect(char** args, t_log *logger) {

//	Convertir args en tipo valido
	e_query queryType = getQueryType(args[0]);
	char *tabla = args[1];
	int key = strtol(args[2], NULL, 10);

//	Paquetizar
	t_package *p = create_package(queryType);
	add_to_package(p, (void*)tabla, sizeof(char) * (strlen(tabla) + 1));
	add_to_package(p, (void*)&key, sizeof(key));

//	obtener memoria segun criterio
	t_memory *mem = get_memory_of_cons_type(CONS_SC);

	//	Enviar query a memoria
	int memSocket = connect_to_memory(mem->ip, mem->port);
	send_package(p, memSocket);

	e_response_code r = recv_res_code(memSocket);

	if(r == RESPONSE_SUCCESS) {
		char *value = recv_str(memSocket);

		FILE * output = txt_open_for_append("../output.txt");
		txt_write_in_file(output, value);
		txt_write_in_file(output, "\n");

	} else {
		close(memSocket);
//		notificar error
		log_error(logger, "Error al realizar query en memoria.");
	}
	return;
}

void qInsert(char** args) {
//	Convertir args en tipo valido
//	Paquetizar
//	Enviar query a memoria
}

void qCreate(char** args) {
//	Convertir args en tipo valido
//	Paquetizar
//	Enviar query a memoria
//	Confirmar creacion de tabla
//	Crear tabla en kernel
}

void qDescribe(char** args) {
//	Convertir args en tipo valido
//	Paquetizar
//	Enviar query a memoria
//	Actualizar estructuras administrativas
}

void qDrop(char** args) {
//	Convertir args en tipo valido
//	Paquetizar
//	Enviar query a memoria
//	Actualizar estructuras administrativas
}
