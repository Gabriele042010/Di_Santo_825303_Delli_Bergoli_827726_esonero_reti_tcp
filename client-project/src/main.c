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

/* Print error messages to stderr */
static void errorhandler(const char *errorMessage)
{
	fprintf(stderr, "%s", errorMessage);
}

int parse_weather_response(const char *input, weather_response_t *resp)
{
	if (!input || !resp)
		return 1;

	unsigned int s;
	char t;
	float v;

	sscanf(input, "%u %c %f", &s, &t, &v);

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
	/* Command line: ./client-project [-s server] [-p port] [-r] "type city"
	   -s server : optional server address (dotted or hostname)
	   -p port   : optional server port
	   -r        : optional flag (if present the next arg may be the request)
	   If -r is not used, the request can be provided as the final positional arg.
	*/

	const char *server_str = SERVER_IP;
	int server_port = SERVER_PORT;
	const char *request_arg = NULL;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-s") == 0)
		{
			if (i + 1 < argc)
			{
				server_str = argv[++i];
				continue;
			}
			fprintf(stderr, "Missing value for -s\n");
			return 1;
		}

		if (strcmp(argv[i], "-p") == 0)
		{
			if (i + 1 < argc)
			{
				server_port = atoi(argv[++i]);
				continue;
			}
			fprintf(stderr, "Missing value for -p\n");
			return 1;
		}

		if (strcmp(argv[i], "-r") == 0)
		{
			if (i + 1 < argc)
			{
				request_arg = argv[++i];
				continue;
			}
			fprintf(stderr, "Missing value for -r\n");
			return 1;
		}

		if (argv[i][0] != '-' && request_arg == NULL)
		{
			request_arg = argv[i];
			continue;
		}
	}

	if (!request_arg)
	{
		fprintf(stderr, "Use: %s [-s server] [-p port] [-r] \"type city\"\n", argv[0]);
		return 1;
	}

	if (server_port <= 0 || server_port > 65535)
	{
		fprintf(stderr, "Invalid port: %d\n", server_port);
		return 1;
	}

#if defined WIN32
	SetConsoleOutputCP(CP_UTF8);
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR)
	{
		errorhandler("Error at WSAStartup() \n");
		return 0;
	}
#endif

	int my_socket;

	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (my_socket < 0)
	{
		errorhandler("Socket creation failed \n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

	unsigned long addr = inet_addr(server_str);

	if (addr == INADDR_NONE)
	{
		/* try DNS lookup */
		struct hostent *he = gethostbyname(server_str);
		if (!he || !he->h_addr_list || !he->h_addr_list[0])
		{
			fprintf(stderr, "Unable to resolve host: %s\n", server_str);
			return 1;
		}
		memcpy(&server_addr.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
	}
	else
	{
		server_addr.sin_addr.s_addr = addr; /* dotted decimal */
	}
	server_addr.sin_port = htons((unsigned short)server_port); // Server port

	if (connect(my_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		errorhandler("Failed to connect.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}
	/* Send the request string (request_arg contains "<type> <city>")
	   Use the real length (including terminating null) and check the return value. */
	int msglen = (int)strlen(request_arg) + 1;
	int sent = send(my_socket, request_arg, msglen, 0);
	 if (sent < 0 || sent != msglen)
	{
		errorhandler("send() failed or sent different number of bytes \n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	weather_request_t req;

	/* Parse the request we sent for later printing */
	parse_weather_request(request_arg, &req);

	//printf("\n type: %c ; city: %s \n", req.type, req.city);		//DEBUG

	/* Receive a single null-terminated response from server */
	int bytes_rcvd;
	char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);

	bytes_rcvd = recv(my_socket, buf, BUFFER_SIZE - 1, 0);
	if (bytes_rcvd <= 0)
	{
		errorhandler("recv() failed or connection closed prematurely \n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}
	buf[bytes_rcvd] = '\0';

	weather_response_t res;
	parse_weather_response(buf, &res);

	//printf("\n status: %u ; type: %c ; value: %.2f \n", res.status, res.type, res.value);		//DEBUG

	switch (res.status)
	{
	case 0:
	{
		printf("Recieved result from server ip %s; %s: ", inet_ntoa(server_addr.sin_addr), req.city);

		switch (res.type)
		{
		case 't':
		{
			printf("Temperature = %.2f°C \n", res.value);
			break;
		}
		case 'h':
		{
			printf("Umidity = %.2f %% \n", res.value);
			break;
		}
		case 'w':
		{
			printf("Wind = %.2f km/h \n", res.value);
			break;
		}
		case 'p':
		{
			printf("Pressure = %.2f hPa \n", res.value);
			break;
		}
		}
		break;
	}

	case 1:
	{
		printf("Recieved result from server ip %s; ", inet_ntoa(server_addr.sin_addr));
		errorhandler(" City not available \n");
		break;
	}

	case 2:
	{
		printf("Recieved result from server ip %s; ", inet_ntoa(server_addr.sin_addr));
		errorhandler("Request not valid \n");
		break;
	}
	}

	closesocket(my_socket);
	puts("Client terminated...");
	clearwinsock();

	return 0;
}
