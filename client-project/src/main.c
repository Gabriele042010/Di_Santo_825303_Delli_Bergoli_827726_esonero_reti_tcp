#if defined WIN32
#include <winsock.h>
#include <windows.h>
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

void clearwinsock()
{
#if defined WIN32
	WSACleanup();
#endif
}

void errorhandler(char *error_message)
{
	printf("%s", error_message);
}

int parse_weather_response(const char *input, weather_response_t *resp)
{
	if (!input || !resp)
		return 1;

	unsigned int s;
	char t;
	float v;

	// sscanf analizza 3 elementi: uint, char, float
	int matched = sscanf(input, "%u %c %f", &s, &t, &v);

	if (matched != 3)
	{
		fprintf(stderr, "Errore: stringa non valida: '%s'\n", input);
		return 1;
	}

	// Controllo sul type
	if (t != 't' && t != 'h' && t != 'w' && t != 'p')
	{
		fprintf(stderr, "Errore: type '%c' non valido.\n", t);
		return 1;
	}

	resp->status = s;
	resp->type = t;
	resp->value = v;

	return 0;
}

void parse_weather_request(const char *input, weather_request_t *req)
{

	// type = primo carattere
	req->type = input[0];

	// trova primo spazio
	const char *space = strchr(input, ' ');
	if (!space || *(space + 1) == '\0')
	{
		return;
	}

	// copia city
	strncpy(req->city, space + 1, sizeof(req->city) - 1);
	req->city[sizeof(req->city) - 1] = '\0';

	return;
}

int main(int argc, char *argv[])
{

	if (argc < 3)
	{
		fprintf(stderr, "Uso: %s <any> <type city>\n", argv[0]);
		return 1;
	}

#if defined WIN32
	SetConsoleOutputCP(CP_UTF8);
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int my_socket;

	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (my_socket < 0)
	{
		errorhandler("socket creation failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // IP del server
	server_addr.sin_port = htons(SERVER_PORT);			// Server port

	if (connect(my_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		errorhandler("Failed to connect.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	 /* Send the request string (argv[2] contains "<type> <city>")
		 Use the real length (including terminating null) and check the return value. */
	 int msglen = (int)strlen(argv[2]) + 1;
	 int sent = send(my_socket, argv[2], msglen, 0);
	 if (sent < 0 || sent != msglen)
	{
		errorhandler("send() failed or sent different number of bytes\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	weather_request_t req;

	/* Parse the request we sent (argv[2]) for later printing */
	parse_weather_request(argv[2], &req);

	/* Receive a single null-terminated response from server */
	int bytes_rcvd;
	char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);

	bytes_rcvd = recv(my_socket, buf, BUFFER_SIZE - 1, 0);
	if (bytes_rcvd <= 0)
	{
		errorhandler("recv() failed or connection closed prematurely");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}
	buf[bytes_rcvd] = '\0';

	weather_response_t res;
	parse_weather_response(buf, &res);

	switch (res.status)
	{
	case 0:
	{
		printf("Ricevuto risultato dal server ip %s %s: ", inet_ntoa(server_addr.sin_addr), req.city);

		switch (res.type)
		{
		case 't':
		{
			printf("Temperatura = %.2f°C \n", res.value);
			break;
		}
		case 'h':
		{
			printf("Umidità = %.2f %% \n", res.value);
			break;
		}
		case 'w':
		{
			printf("Vento = %.2f km/h \n", res.value);
			break;
		}
		case 'p':
		{
			printf("Pressione = %.2f hPa \n", res.value);
			break;
		}
		}
		break;
	}

	case 1:
	{
		puts("Città non disponibile");
		break;
	}

	case 2:
	{
		puts("Richiesta non valida");
		break;
	}
	}

	closesocket(my_socket);
	printf("Client terminated.\n");
	clearwinsock();

	return 0;
}
