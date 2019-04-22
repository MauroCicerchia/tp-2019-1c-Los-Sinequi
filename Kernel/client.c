#include"client.h"

int connectToServer() {
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	return serverSocket;
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
