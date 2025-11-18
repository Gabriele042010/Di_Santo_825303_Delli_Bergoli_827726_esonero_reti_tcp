/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
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

void errorhandler(char *error_message) {
printf("%s",error_message);
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

	int my_socket;

	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (my_socket < 0) {
		errorhandler("socket creation failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP del server
	server_addr.sin_port = htons(SERVER_PORT); // Server port


	if (connect(my_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))< 0)
	{
		errorhandler( "Failed to connect.\n" );
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}





	closesocket(my_socket);
	printf("Client terminated.\n");
	clearwinsock();

	return 0;
} // main end
