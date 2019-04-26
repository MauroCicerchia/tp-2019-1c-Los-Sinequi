#include"server.h"

int createServer() {

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo);

	int listeningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(listeningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	listen(listeningSocket, BACKLOG);

	return listeningSocket;
}

int connectToClient(int listeningSocket) {
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	int client = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);

	return client;
}

int readQueryFromClient(int client, char *package) {

	int status = recv(client, (void*) package, PACKAGESIZE, 0);

	if(strcasecmp("exit\n", package) == 0) return -1;

	return status;

}

void closeServer(int server) {
	close(server);
}
