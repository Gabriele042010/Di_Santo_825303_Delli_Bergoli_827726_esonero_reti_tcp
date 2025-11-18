/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */
//TODO ripulire il codice e ottimizzare

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


int parse_weather_response(const char *input, weather_response_t *resp) {
    if (!input || !resp) return 1;

    unsigned int s;
    char t;
    float v;

    // sscanf analizza 3 elementi: uint, char, float
    int matched = sscanf(input, "%u %c %f", &s, &t, &v);

    if (matched != 3) {
        fprintf(stderr, "Errore: stringa non valida: '%s'\n", input);
        return 1;
    }

    // Controllo sul type
    if (t != 't' && t != 'h' && t != 'w' && t != 'p') {
        fprintf(stderr, "Errore: type '%c' non valido.\n", t);
        return 1;
    }

    resp->status = s;
    resp->type   = t;
    resp->value  = v;

    return 0;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <type city>\n", argv[0]);
        return 1;
    }

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
	server_addr.sin_addr.s_addr = inet_addr("10.192.195.73"); // IP del server
	server_addr.sin_port = htons(SERVER_PORT); // Server port


	if (connect(my_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))< 0)
	{
		errorhandler( "Failed to connect.\n" );
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}



	if(send(my_socket, argv[2], BUFFER_SIZE))
	{
	errorhandler("send() sent a different number of byte than expected");
	closesocket(my_socket);
	clearwinsock();
	return -1;
	}

	int bytes_rcvd;
	int total_bytes_rcvd = 0;
	char buf[BUFFER_SIZE]; // buffer for data from the server
	memset(buf, 0, BUFFER_SIZE); // ensures extra bytes contain 0
	printf("Received: "); // Setup to print the echoed string

	while (total_bytes_rcvd < BUFFER_SIZE) {
	if ((bytes_rcvd = recv(my_socket, buf, BUFFER_SIZE - 1, 0)) <= 0)
	{
	errorhandler("recv() failed or connection closed prematurely");
	closesocket(my_socket);
	clearwinsock();
	return -1;
	}
	total_bytes_rcvd += bytes_rcvd; // Keep tally of total bytes
	}

	weather_response_t res;
	//funz di conversione in weather response e stampa dei risultati
	parse_weather_response(&buf, &res);

	//stampa

	closesocket(my_socket);
	printf("Client terminated.\n");
	clearwinsock();

	return 0;
} // main end
