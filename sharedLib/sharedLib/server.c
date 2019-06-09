#include"server.h"

int createServer(char* IP, char *PORT) {

	int server_socket;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, PORT, &hints, &servinfo);

	for (p=servinfo; p != NULL; p = p->ai_next)
	{
		if ((server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (bind(server_socket, p->ai_addr, p->ai_addrlen) == -1) {
			close(server_socket);
			continue;
		}
		break;
	}

	listen(server_socket, SOMAXCONN);

	freeaddrinfo(servinfo);

	return server_socket;
}

int connectToClient(int listeningSocket) {
	struct sockaddr_in client_addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);

	int client_socket = accept(listeningSocket, (void*) &client_addr, &addr_size);
	if(client_socket < 0) {
		perror("accept error : ");
	}

	return client_socket;
}

int readQueryFromClient(int client, char *package) {

	int status = recv(client, (void*) package, PACKAGESIZE, 0);
	if(strcasecmp("exit\n", package) == 0) return -1;

	return status;

}

void closeServer(int server) {
	close(server);
}
