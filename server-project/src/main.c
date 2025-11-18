/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
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
#include <time.h>
#include "protocol.h"

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void errorhandler(char *errorMessage) {
printf ("%s", errorMessage);
}

int stricmp_ci(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

int is_valid_city(const char *city) {
    const char *valid_cities[] = {
        "Bari", "Roma", "Milano", "Napoli", "Torino",
        "Palermo", "Genova", "Bologna", "Firenze", "Venezia"
    };
    const int n = sizeof(valid_cities) / sizeof(valid_cities[0]);

    for (int i = 0; i < n; i++) {
        if (stricmp_ci(city, valid_cities[i])) {
            return 1;
        }
    }
    return 0;
}

int parse_weather_request(const char *input, weather_request_t *req) {
    if (!input || !req) return 2;

    // type = primo carattere
    req->type = input[0];

    // --- Controllo sul type ---
    if (req->type != 't' &&
        req->type != 'h' &&
        req->type != 'w' &&
        req->type != 'p') {
        return 2;
    }

    // trova primo spazio
    const char *space = strchr(input, ' ');
    if (!space || *(space + 1) == '\0') {
        return 2;
    }

    // copia city
    strncpy(req->city, space + 1, sizeof(req->city) - 1);
    req->city[sizeof(req->city) - 1] = '\0';

    // controllo validità città
    if (!is_valid_city(req->city)) {
        return 1;
    }

    return 0;
}

/// @brief Converte la struct in una stringa: "<status> <type> <value>"
/// @param resp struct con i dati
/// @param buffer stringa di output
/// @param size dimensione del buffer
/// @return 0 se ok, 1 se errore
int format_weather_response(const weather_response_t *resp, char *buffer, size_t size) {
    if (!resp || !buffer || size == 0) return 1;

    int written = snprintf(buffer, size, "%u %c %.2f",
                           resp->status,
                           resp->type,
                           resp->value);

    // snprintf ritorna i caratteri che *avrebbe* scritto
    if (written < 0 || (size_t)written >= size) {
        return 1; // buffer troppo piccolo
    }

    return 0;
}


// Da chiamare UNA sola volta nel main
void init_random(void) {
    srand((unsigned int)time(NULL));
}

// Genera un float in un intervallo [min, max]
static float rand_range(float min, float max) {
    float r = (float)rand() / (float)RAND_MAX;  // 0.0 → 1.0
    return min + r * (max - min);
}

float get_temperature(void) {
    return rand_range(-10.0f, 40.0f);
}

float get_humidity(void) {
    return rand_range(20.0f, 100.0f);
}

float get_wind(void) {
    return rand_range(0.0f, 100.0f);
}

float get_pressure(void) {
    return rand_range(950.0f, 1050.0f);
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


	char data[BUFFER_SIZE];
	memset(data, 0, BUFFER_SIZE); // ensures extra bytes contain 0

	int bytes_rcvd;
	int total_bytes_rcvd = 0;

	while (total_bytes_rcvd < BUFFER_SIZE) {
	if ((bytes_rcvd = recv(client_socket, data, BUFFER_SIZE - 1, 0)) <= 0)
	{
	errorhandler("recv() failed or connection closed prematurely");
	closesocket(client_socket);
	clearwinsock();
	return -1;
	}
	total_bytes_rcvd += bytes_rcvd; // Keep tally of total bytes
	}

	weather_request_t req;
	weather_response_t res;

	    switch (parse_weather_request(data, &req)){
	    	case 0:{
	    	res.status = 0;
	    	res.type = req.type;

	    	switch(req.type){
	    	case 't':{
	    		res.value = get_temperature();
	    		break;
	    	}
	    	case 'h':{
	    		res.value= get_humidity();
	    		break;
	    	}
	    	case 'w':{
	    		res.value = get_wind();
	    		break;
	    	}
	    	case 'p':{
	    		res.value= get_pressure();
	    		break;
	    	}
	    	}

	    	char data[BUFFER_SIZE];

	    	format_weather_response(&res, data, BUFFER_SIZE);

	    	if(send(client_socket, data, BUFFER_SIZE, 0))
	    	{
	    	errorhandler("send() sent a different number of byte than expected");
	    	closesocket(client_socket);
	    	clearwinsock();
	    	return -1;
	    	}

	    	break;
	    	}

	    	case 1:{
	    	res.status=1;
	    	break;
	    	}
	    	case 2:{
	    	res.status = 2;
	    	break;
	    	}
	    }

	}// end of the while loop

	printf("Server terminated.\n");

	closesocket(my_socket);
	clearwinsock();

	return 0;
} // main end
