/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void errorhandler(char *errorMessage) {
printf ("%s", errorMessage);
}

int main(int argc, char *argv[]) {

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif


	int my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (my_socket < 0) {
		errorhandler("socket creation failed.\n");
		clearwinsock();
		return -1;
	}

	//Assegnazione indirizzo alla socket
	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(my_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		errorhandler("bind() failed.\n");
		closesocket(my_socket);
		clearwinsock();
	return -1;
	}

	//Socket in ascolto
	if (listen (my_socket, QUEUE_SIZE) < 0) {
		errorhandler("listen() failed.\n");
		closesocket(my_socket);
		clearwinsock();
	 return -1;
	}

	//accettazione nuova connessione
	struct sockaddr_in cad; //structure for the client address
	int client_socket; //socket descriptor for the client
	int client_len; //the size of the client address
	puts("Waiting for a client to connect...");

	while (1) {
		client_len = sizeof(cad); //set the size of the client address
		if ( (client_socket=accept(my_socket, (struct sockaddr *)&cad, &client_len)) < 0 ) {
			errorhandler("accept() failed.\n");
			closesocket(client_socket);
			clearwinsock();
			return -1;
		}
	// clientSocket is connected to a client
		printf( "Handling client %s\n", inet_ntoa(cad.sin_addr) );




	}// end of the while loop

	printf("Server terminated.\n");

	closesocket(my_socket);
	clearwinsock();

	return 0;
} // main end
