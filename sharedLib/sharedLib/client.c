#include"client.h"

int connectToServer(char *IP, char *PORT) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, PORT, &hints, &server_info);

	int client_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(client_socket, server_info->ai_addr, server_info->ai_addrlen) == -1){
//		printf("Error al conectarse al servidor. Try again later.\n");
		freeaddrinfo(server_info); //ADROINFO
		close(client_socket);
		return -1;
	}
	freeaddrinfo(server_info);

	return client_socket;
}

void sendMessage(int serverSocket,char *message){
	send(serverSocket, message, strlen(message) + 1, 0);
}

void sendMessages(int serverSocket) {
	int enviar = 1;
	char message[PACKAGESIZE];

	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while(enviar){
		printf(">");
		fgets(message, PACKAGESIZE, stdin);
		send(serverSocket, message, strlen(message) + 1, 0);
		if(strcasecmp("exit\n", message) == 0) enviar = 0;
	}
}

void closeConnection(int serverSocket) {
	close(serverSocket);
}
